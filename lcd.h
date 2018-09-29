// LCD module control primitive
void lcd_init(void);                // must call this first
void lcd_cls(void);
void lcd_goto(int col, int line);
void lcd_putc(char c);
void lcd_puts(char *s);
