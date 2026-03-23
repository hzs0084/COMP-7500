/******************************************************************************
 * File: cpmfsys.c
 * Author: Hemant Sherawat
 * Course: COMP 7500 - Operating Systems
 * Project: Project 4 - cpmFS (Simple File System)
 *
 * Description:
 * This file implements a simplified CP/M-style file system. It provides
 * functionality for managing directory entries, maintaining a free block list,
 * and performing file operations such as listing, deleting, and renaming files.
 *
 * The file system operates on a simulated disk using block-based storage.
 *
 * Key Features:
 * - Directory parsing and reconstruction from disk
 * - Free block list generation
 * - File listing with size computation
 * - File deletion and block reclamation
 * - File renaming with validation
 *
 * Notes:
 * - The directory is stored in block 0
 * - Each extent is 32 bytes
 * - File names follow 8.3 format
 *
 ******************************************************************************/

#include "cpmfsys.h"
#include "diskSimulator.h"
#include <ctype.h>

bool freeList[NUM_BLOCKS];

/*
 * trimCopyField
 * -------------
 * Copies a fixed-length CP/M name or extension field into a C string.
 * Trims trailing spaces and masks the high bit of each character because
 * CP/M directory entries may store attribute information in the high bit.
 */

static void trimCopyField(char *dest, const uint8_t *src, int len) {
    int i, last = -1;

    for (i = 0; i < len; i++) {
        char c = (char)(src[i] & 0x7F);   // mask high bit
        if (c != ' ')
            last = i;
    }

    if (last == -1) {
        dest[0] = '\0';
        return;
    }

    for (i = 0; i <= last; i++) {
        dest[i] = (char)(src[i] & 0x7F);  // mask high bit
    }
    dest[last + 1] = '\0';
}

/*
 * writePaddedField
 * ----------------
 * Writes a C string into a fixed-length CP/M directory field.
 * Any unused bytes are padded with spaces to preserve on-disk 8.3 format.
 */

static void writePaddedField(uint8_t *dest, const char *src, int len) {
    int i;

    for (i = 0; i < len; i++) {
        dest[i] = ' ';
    }

    for (i = 0; i < len && src[i] != '\0'; i++) {
        dest[i] = (uint8_t)src[i];
    }
}

/*
 * splitName
 * ---------
 * Splits a filename in 8.3 form into separate base-name and extension
 * components. The base name is limited to 8 characters and the extension
 * is limited to 3 characters.
 */

static void splitName(const char *fullName, char *namePart, char *extPart) {
    const char *dot = strchr(fullName, '.');
    size_t nameLen, extLen;

    if (dot == NULL) {
        strncpy(namePart, fullName, 8);
        namePart[8] = '\0';
        extPart[0] = '\0';
        return;
    }

    nameLen = (size_t)(dot - fullName);
    if (nameLen > 8) nameLen = 8;

    memcpy(namePart, fullName, nameLen);
    namePart[nameLen] = '\0';

    extLen = strlen(dot + 1);
    if (extLen > 3) extLen = 3;

    memcpy(extPart, dot + 1, extLen);
    extPart[extLen] = '\0';
}

/*
 * countUsedBlocks
 * ---------------
 * Counts how many data block entries in a directory extent are in use.
 * A block entry of 0 is treated as unused.
 */

static int countUsedBlocks(const DirStructType *d) {
  int count = 0;
  for (int i = 0; i < BLOCKS_PER_EXTENT; i++) {
    if (d->blocks[i] != 0) {
      count++;
    }
  }
  return count;
}

/*
 * fileSize
 * --------
 * Computes the file size in bytes using the number of allocated blocks
 * and the CP/M metadata fields RC and BC for the final partial block.
 */

static int fileSize(const DirStructType *d) {
  int usedBlocks = countUsedBlocks(d);
  if (usedBlocks == 0) {
    return 0;
  }
  return (usedBlocks - 1) * BLOCK_SIZE + d->RC * 128 + d->BC;
}

/*
 * mkDirStruct
 * -----------
 * Converts a raw 32-byte directory extent into a structured in-memory object.
 * Masks high bits and trims padding spaces from filename and extension.
 */

DirStructType *mkDirStruct(int index, uint8_t *e) {
    int offset = index * EXTENT_SIZE;
    DirStructType *d = malloc(sizeof(DirStructType));
    if (d == NULL) return NULL;

    d->status = e[offset];
    trimCopyField(d->name, e + offset + 1, 8);
    trimCopyField(d->extension, e + offset + 9, 3);
    d->XL = e[offset + 12];
    d->BC = e[offset + 13];
    d->XH = e[offset + 14];
    d->RC = e[offset + 15];

    for (int i = 0; i < BLOCKS_PER_EXTENT; i++) {
        d->blocks[i] = e[offset + 16 + i];
    }

    return d;
}

/*
 * writeDirStruct
 * --------------
 * Writes an in-memory directory structure back to its corresponding
 * 32-byte extent in the directory block (block 0).
 *
 * This function converts the structured representation of a file
 * (DirStructType) into the raw CP/M on-disk format by:
 *  - Writing the status byte
 *  - Writing the filename (8 bytes, space-padded)
 *  - Writing the extension (3 bytes, space-padded)
 *  - Copying metadata fields (XL, BC, XH, RC)
 *  - Writing the block allocation list
 *
 * The function ensures that all fields are properly formatted according
 * to CP/M conventions, including fixed-length fields and space padding.
 *
 * Parameters:
 *  d      - Pointer to the directory structure to write
 *  index  - Directory entry index (0–31) within block 0
 *  e      - Buffer containing the raw directory block (block 0)
 *
 * Notes:
 *  - This function modifies the in-memory block buffer; the caller is
 *    responsible for writing the buffer back to disk using blockWrite().
 */

void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e) {
    int offset = index * EXTENT_SIZE;

    e[offset] = d->status;
    writePaddedField(e + offset + 1, d->name, 8);
    writePaddedField(e + offset + 9, d->extension, 3);
    e[offset + 12] = d->XL;
    e[offset + 13] = d->BC;
    e[offset + 14] = d->XH;
    e[offset + 15] = d->RC;

    for (int i = 0; i < BLOCKS_PER_EXTENT; i++) {
        e[offset + 16 + i] = d->blocks[i];
    }
}

/*
 * makeFreeList
 * ------------
 * Builds the free block list by scanning all directory entries.
 * Marks block 0 (directory) and all blocks referenced by active files as used.
 */

void makeFreeList() {
  uint8_t block0[BLOCK_SIZE];
  blockRead(block0, 0);

  for (int i = 0; i < NUM_BLOCKS; i++) {
    freeList[i] = true;
  }
  freeList[0] = false;

  for (int i = 0; i < BLOCK_SIZE / EXTENT_SIZE; i++) {
    DirStructType *d = mkDirStruct(i, block0);
    if (d == NULL) {
      continue;
    }

    if (d->status != 0xe5) {
      for (int j = 0; j < BLOCKS_PER_EXTENT; j++) {
        if (d->blocks[j] != 0) {
          freeList[d->blocks[j]] = false;
        }
      }
    }
    free(d);
  }
}

/*
 * printFreeList
 * -------------
 * Displays the free block list in a formatted 16x16 grid.
 * '*' indicates a used block, '.' indicates a free block.
 */

void printFreeList() {
  fprintf(stdout, "FREE BLOCK LIST: (* means in-use)\n");

  for (int row = 0; row < 16; row++) {

    // EXACT formatting: 2-wide hex with leading space
    fprintf(stdout, "%3x: ", row * 16);

    for (int col = 0; col < 16; col++) {
      int block = row * 16 + col;
      fprintf(stdout, "%c ", freeList[block] ? '.' : '*');
    }

    // DO NOT remove trailing space before newline
    fprintf(stdout, "\n");
  }
}


/*
 * cpmDir
 * ------
 * Lists all active files in the directory along with their sizes.
 * File size is computed based on block allocation and metadata.
 */

void cpmDir() {
  uint8_t block0[BLOCK_SIZE];
  blockRead(block0, 0);
  fprintf(stdout, "DIRECTORY LISTING\n");

  for (int i = 0; i < BLOCK_SIZE / EXTENT_SIZE; i++) {
    DirStructType *d = mkDirStruct(i, block0);
    if (d == NULL) {
      continue;
    }
    if (d->status != 0xe5) {
      fprintf(stdout, "%s.%s %d\n", d->name, d->extension, fileSize(d));
    }
    free(d);
  }
}


/*
 * checkLegalName
 * --------------
 * Validates that a filename follows the CP/M 8.3 format:
 * - Max 8 characters for name
 * - Max 3 characters for extension
 * - Only alphanumeric characters allowed
 */

bool checkLegalName(char *name) {
  int dotCount = 0;
  int nameLen = 0;
  int extLen = 0;
  bool inExt = false;

  if (name == NULL || name[0] == '\0') {
    return false;
  }

  for (int i = 0; name[i] != '\0'; i++) {
    if (name[i] == '.') {
      dotCount++;
      if (dotCount > 1 || i == 0 || inExt) {
        return false;
      }
      inExt = true;
      continue;
    }

    if (!isalnum((unsigned char)name[i])) {
      return false;
    }

    if (!inExt) {
      nameLen++;
      if (nameLen > 8) {
        return false;
      }
    } else {
      extLen++;
      if (extLen > 3) {
        return false;
      }
    }
  }

  return nameLen >= 1;
}

/*
 * findExtentWithName
 * ------------------
 * Searches block 0 for an active directory extent whose filename and
 * extension match the requested 8.3 name. Returns the extent index if
 * found, or -1 if no matching file exists.
 */

int findExtentWithName(char *name, uint8_t *block0) {
  char wantedName[9];
  char wantedExt[4];

  if (!checkLegalName(name)) {
    return -1;
  }

  splitName(name, wantedName, wantedExt);

  for (int i = 0; i < BLOCK_SIZE / EXTENT_SIZE; i++) {
    DirStructType *d = mkDirStruct(i, block0);
    if (d == NULL) {
      continue;
    }

    if (d->status != 0xe5 &&
        strcmp(d->name, wantedName) == 0 &&
        strcmp(d->extension, wantedExt) == 0) {
      free(d);
      return i;
    }

    free(d);
  }

  return -1;
}

/*
 * cpmDelete
 * ---------
 * Deletes a file by:
 * - Marking its directory entry as unused (0xE5)
 * - Freeing all associated data blocks
 */

int cpmDelete(char *fileName) {
  uint8_t block0[BLOCK_SIZE];
  int extentIndex;

  if (!checkLegalName(fileName)) {
    return -2;
  }

  blockRead(block0, 0);
  extentIndex = findExtentWithName(fileName, block0);
  if (extentIndex < 0) {
    return -1;
  }

  DirStructType *d = mkDirStruct(extentIndex, block0);
  if (d == NULL) {
    return -1;
  }

  for (int i = 0; i < BLOCKS_PER_EXTENT; i++) {
    if (d->blocks[i] != 0) {
      freeList[d->blocks[i]] = true;
      d->blocks[i] = 0;
    }
  }

  d->status = 0xe5;
  d->name[0] = '\0';
  d->extension[0] = '\0';
  d->XL = 0;
  d->BC = 0;
  d->XH = 0;
  d->RC = 0;

  writeDirStruct(d, (uint8_t)extentIndex, block0);
  blockWrite(block0, 0);
  free(d);
  return 0;
}


/*
 * cpmRename
 * ---------
 * Renames a file by updating its directory entry.
 * Ensures the new name is valid and does not already exist.
 */

int cpmRename(char *oldName, char *newName) {
  uint8_t block0[BLOCK_SIZE];
  int extentIndex;
  char newBase[9];
  char newExt[4];

  if (!checkLegalName(newName)) {
    return -2;
  }

  blockRead(block0, 0);
  extentIndex = findExtentWithName(oldName, block0);
  if (extentIndex < 0) {
    return -1;
  }

  if (findExtentWithName(newName, block0) >= 0) {
    return -3;
  }

  splitName(newName, newBase, newExt);

  DirStructType *d = mkDirStruct(extentIndex, block0);
  if (d == NULL) {
    return -1;
  }

  strncpy(d->name, newBase, 8);
  d->name[8] = '\0';

  strncpy(d->extension, newExt, 3);
  d->extension[3] = '\0';
  writeDirStruct(d, (uint8_t)extentIndex, block0);
  blockWrite(block0, 0);
  free(d);
  return 0;
}