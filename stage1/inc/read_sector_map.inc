
map_temp_addr = 0x1000                /* Where to load temporary sector map chunks */
map_next_addr = map_temp_addr + 0x1fc /* Location of pointer to next sector - if we get here, we may need to read again. */

/* Reads through entire sector map chain */
read_sector_map:
    movw (bootldr.stage2_map_sector), %ax /* First map sector */
    movw $map_temp_addr,              %bx /* Load to temporary address */
    call read_sector

    pusha
    movw (bootldr.stage2_addr), %bx
    movw $map_temp_addr, %si
  .handle_map_entry:
    movw 0(%si), %ax    /* Sector to read */
    test %ax, %ax
    jz   .map_chain_end /* Sector of 0 signals end-of-map */
    movw 2(%si), %cx    /* Sector count */

  .do_map_read:
    call read_sector
    addw $512, %bx      /* Next destination address */
    incw %ax            /* Next sector */
    decw %cx
    jnz .do_map_read

    addw $0x04, %si     /* Move to next entry */
    cmp  $map_next_addr, %dx
    jge  .map_end       /* We've reached the end of this map sector */

    jmp .handle_map_entry

  .map_end:
    movw (%si), %si                       /* Read pointer to next sector map */
    test %si, %si
    jz   .map_chain_end                   /* Sector of 0 signals end-of-map */

    movw %si, (bootldr.stage2_map_sector) /* Update next map sector */
    movw %bx, (bootldr.stage2_addr)       /* Save current position */

    popa
    jmp read_sector_map                   /* Read next map set */

  .map_chain_end:
    popa
    ret
