#include "mdadm.h"
#include "jbod.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

int mounted = 0;

int mdadm_mount(void) {
  if (mounted == 1) {
    return -1;
  }
  uint32_t op = (JBOD_MOUNT << 12);
  int result = jbod_operation(op, NULL);
  if (result == 0) {
    mounted = 1;
    return 1;
  }
  return -1;
}

int mdadm_unmount(void) {
  if (mounted == 0) {
    return -1;
  }
  uint32_t op = (JBOD_UNMOUNT << 12);
  int result = jbod_operation(op, NULL);
  if (result == 0) {
    mounted = 0;
    return 1;
  }
  return -1;
}

int mdadm_read(uint32_t start_address, uint32_t read_length,
               uint8_t *read_buffer) {
  if (mounted == 0) {
    return -3;
  }

  // Check Parames
  if (start_address + read_length > JBOD_DISK_SIZE * JBOD_NUM_DISKS) {
    return -1;
  }
  if (read_length > 1024) {
    return -2;
  }
  // Check Buffer
  if (read_length > 0 && read_buffer == NULL) {
    return -4;
  }

  uint32_t bytes_read = 0;
  uint32_t current_address = start_address;

  while (bytes_read < read_length) {

    uint32_t disk_id = current_address / JBOD_DISK_SIZE;
    uint32_t block_id = (current_address % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
    uint32_t offset = (current_address % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;

    uint32_t op = (disk_id & 0xFFF) | (JBOD_SEEK_TO_DISK << 12);

    if (jbod_operation(op, NULL) == -1) {
      return -1;
    }

    // Seek to the correct block
    op = (JBOD_SEEK_TO_BLOCK << 12) |
         (block_id & 0xFF) << 4; // Ensure block_id fits in the relevant bits
    if (jbod_operation(op, NULL) == -1) {
      return -1;
    }

    // Read the block
    uint8_t block[JBOD_BLOCK_SIZE]; // Buffer to hold the block data
    op = (JBOD_READ_BLOCK << 12);
    if (jbod_operation(op, block) == -1) {
      return -1; // Error reading block
    }

    uint32_t bytes_to_copy_from_block = JBOD_BLOCK_SIZE - offset;
    if (bytes_to_copy_from_block > read_length - bytes_read) {
      bytes_to_copy_from_block = read_length - bytes_read;
    }

    memcpy(read_buffer + bytes_read, block + offset, bytes_to_copy_from_block);

    bytes_read += bytes_to_copy_from_block;
    current_address += bytes_to_copy_from_block;
  }

  return bytes_read;
}
