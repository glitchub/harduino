// Read or write eeprom. A write operation takes about 3.3mS, functions will
// block while a previous write is in progress.
void write_eeprom(uint16_t offset, uint8_t byte)
{
    while (!eeprom_is_ready()) yield();
    eeprom_update_byte((uint8_t *)offset, byte);
}

uint8_t read_eeprom(uint16_t offset)
{
    while (!eeprom_is_ready()) yield();
    return eeprom_read_byte((uint8_t *)offset);
}

// Valid param bytes, the actual value is the low nibble
static const uint8_t params[] = {0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x87, 0x78, 0x69, 0x5A, 0x4B, 0x3C, 0x2D, 0x1E, 0x0F};

// Store param 0-15 to specified offset.
void write_eeparam(uint16_t offset, uint8_t param)
{
    write_eeprom(offset, params[param & 15]);
}

// Read param from specified offset, if valid update variable and return true.
// If invalid (likely unprogrammed), just return false.
bool read_eeparam(uint16_t offset, uint8_t *param)
{
    uint8_t b = read_eeprom(offset);
    if (b != params[b & 15]) return false;
    *param=b & 15;
    return true;
}

#ifdef COMMAND
COMMAND(eeprom, NULL, "read/write eeprom")
{
    if (argc < 2 || argc > 3) die("Usage: eeprom offset [byte]");
    uint16_t offset = strtoul(argv[1],NULL,0);
    uint8_t byte;
    if (argc == 3)
    {
        byte = strtoul(argv[2],NULL,0);
        write_eeprom(offset, byte);
    } else
        byte = read_eeprom(offset);
    pprintf("%s eeprom %04X = %02X\n", (argc==3)?"Wrote":"Read", offset, byte);
}
#endif
