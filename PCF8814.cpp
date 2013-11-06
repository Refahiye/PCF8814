//***************************************************************************
//  File........: PCF8814.h
//  Author(s)...: Chiper
//  Porting.....: Igorok107
//  URL(s)......: http://digitalchip.ru
//  Description.: ������� LCD-����������� �� Nokia1100 � ������������ ���������
//  Data........: 02.11.13
//  Version.....: 2.1.0
//***************************************************************************
#include "PCF8814.h"
#include "PCF8814_font.h" // ���������� ����� (����� �������� � ����������� ������)

// ����������. �������� ����� ������, ��� ��� �� ����������� ������ ������ ������, � ���
// ������������ ������ ��� ����� ����� ���������� �����������. (9 ������ �� 96 ����)
static byte lcd_memory[LCD_X_RES][(LCD_Y_RES/8)+1];

// ������� ���������� (���������) � �����������
// lcd_xcurr - � ��������, lcd_ycurr- � ������ (�������)
static byte lcd_xcurr, lcd_ycurr;

// ����� � ������� ��������� ������� � ��������� Arduino
volatile byte LCD_SCLK, LCD_SDA, LCD_CS, LCD_RST;

PCF8814::PCF8814(byte _LCD_SCLK, byte _LCD_SDA, byte _LCD_CS, byte _LCD_RST)
{
	// �������������� ���� �� ����� ��� ������ � LCD-������������
	pinMode(_LCD_SCLK,OUTPUT);
	pinMode(_LCD_SDA,OUTPUT);
	pinMode(_LCD_CS,OUTPUT);
	pinMode(_LCD_RST,OUTPUT);
  
	LCD_SCLK	=	_LCD_SCLK;
	LCD_SDA		=	_LCD_SDA;
	LCD_CS		=	_LCD_CS;
	LCD_RST		=	_LCD_RST;
}

//******************************************************************************
// �������� ����� (������� ��� ������) �� LCD-����������
//  mode: CMD_LCD_MODE - �������� �������
//		  DATA_LCD_MODE - �������� ������
//  c: �������� ������������� �����
void PCF8814::SendByte(char mode,unsigned char c)
{
  CS_LCD_RESET;
  SCLK_LCD_RESET;
  if (mode)
  {
    lcd_memory[lcd_xcurr][lcd_ycurr] = c;
    lcd_xcurr++;
    if (lcd_xcurr>95)
    {
      lcd_xcurr = 0;
      lcd_ycurr++;
    }
    if (lcd_ycurr>8) lcd_ycurr = 0;			
    SDA_LCD_SET;
  }
  else SDA_LCD_RESET;

  SCLK_LCD_SET;

  byte i;
  for(i=0;i<8;i++)
  {
    delayMicroseconds(LCD_MIN_DELAY/2);
    SCLK_LCD_RESET;

    if(c & 0x80) SDA_LCD_SET;
    else	     SDA_LCD_RESET;
    delayMicroseconds(LCD_MIN_DELAY/2);
    SCLK_LCD_SET;
    c <<= 1;	
  }	
  CS_LCD_SET;
  SCLK_LCD_RESET;
  SDA_LCD_RESET;
}

//******************************************************************************
// ������������� �����������
void PCF8814::Init(void)
{
  SCLK_LCD_RESET;
  SDA_LCD_RESET;
  CS_LCD_RESET;
  RST_LCD_RESET;
  delay(10);            // ������� �� ����� 5�� ��� ��������� ����������(����� 5 �� ����� ����������)
  RST_LCD_SET;

  SendByte(CMD_LCD_MODE,0xE2); // *** SOFTWARE RESET 
  SendByte(CMD_LCD_MODE,0x3A); // *** Use internal oscillator
  SendByte(CMD_LCD_MODE,0xEF); // *** FRAME FREQUENCY:
  SendByte(CMD_LCD_MODE,0x04); // *** 80Hz
  SendByte(CMD_LCD_MODE,0xD0); // *** 1:65 divider
  SendByte(CMD_LCD_MODE,0x20); // ������ � ������� Vop 
  SendByte(CMD_LCD_MODE,0x85); // ���������� �������������
  SendByte(CMD_LCD_MODE,0xA4); // all on/normal display
  SendByte(CMD_LCD_MODE,0x2F); // Power control set(charge pump on/off)
  SendByte(CMD_LCD_MODE,0x40); // set start row address = 0
  SendByte(CMD_LCD_MODE,0xB0); // ���������� Y-����� = 0
  SendByte(CMD_LCD_MODE,0x10); // ���������� X-�����, ������� 3 ����
  SendByte(CMD_LCD_MODE,0x00);  // ���������� X-�����, ������� 4 ����

  //SendByte(CMD_LCD_MODE,0xC8); // mirror Y axis (about X axis)
  SendByte(CMD_LCD_MODE,0xA1); // ������������� ����� �� �����������

  SendByte(CMD_LCD_MODE,0xAC); // set initial row (R0) of the display
  SendByte(CMD_LCD_MODE,0x07);
  SendByte(CMD_LCD_MODE,0xAF); // ����� ���/����

  Clear(); // clear LCD
}

//******************************************************************************
// ������� ������
void PCF8814::Clear(void)
{
  SendByte(CMD_LCD_MODE,0x40); // Y = 0
  SendByte(CMD_LCD_MODE,0xB0);
  SendByte(CMD_LCD_MODE,0x10); // X = 0
  SendByte(CMD_LCD_MODE,0x00);

  lcd_xcurr=0; 
  lcd_ycurr=0;		  // ������������� � 0 ������� ���������� � �����������

  SendByte(CMD_LCD_MODE,0xAE); // disable display;
  unsigned int i;
  for(i=0;i<864;i++) SendByte(DATA_LCD_MODE,0x00);
  SendByte(CMD_LCD_MODE,0xAF); // enable display;
}

//******************************************************************************
// �������������� LCD-������ �� ��� x � y ��������������.
//  ON: ��������
//  OFF: �� ���������
void PCF8814::Mirror(byte x, byte y)
{
	SendByte(CMD_LCD_MODE,0xA0 | x);
	SendByte(CMD_LCD_MODE,0xC0 | y<<3);
}

//******************************************************************************
// ������������� LCD-������.
//  �: ��������� �������� �� 0 �� 31.
void PCF8814::Contrast(byte c)
{
	if (c >= 0x20) c = 0x1F;
		SendByte(CMD_LCD_MODE,0x20);
		SendByte(CMD_LCD_MODE,0x80+c); // ���������� ������������� [0x80-0x9F]
}

//******************************************************************************
// ����� ������� �� LCD-����� � ������� �����
//  c: ��� �������
void PCF8814::Putc(unsigned char c)
{
  if (c < 208){
    byte i;
    for ( i = 0; i < 5; i++ )
      SendByte(DATA_LCD_MODE,pgm_read_byte(&(lcd_Font[c-32][i])));

    SendByte(DATA_LCD_MODE,0x00); // ����� ����� ��������� �� ����������� � 1 �������
  }
}

//******************************************************************************
// ����� �������� ������� �� LCD-����� � ������� �����
//  c: ��� �������
void PCF8814::PutcWide(unsigned char c)
{
  if (c < 208){ 	// ������� ������ ���� � ��������� UTF8

    byte i;
    for ( i = 0; i < 5; i++ )
    {
      unsigned char glyph = pgm_read_byte(&(lcd_Font[c-32][i]));
      SendByte(DATA_LCD_MODE,glyph);
      SendByte(DATA_LCD_MODE,glyph);
    }

    SendByte(DATA_LCD_MODE,0x00); // ����� ����� ��������� �� ����������� � 1 �������
    //	SendByte(DATA_LCD_MODE,0x00); // ����� ������� ��� �����
  }
}

//******************************************************************************
// ����� ������ �������� �� LCD-����� � ������� �����. ���� ������ �������
// �� ����� � ������� ������, �� ������� ����������� �� ��������� ������.
//  message: ��������� �� ������ ��������. 0x00 - ������� ����� ������.
void PCF8814::Print(const char * message)
{
  while (*message) Putc(*message++); // ����� ������ ��������� �����
}

//******************************************************************************
// ����� ������ �������� ������� ������ �� LCD-����� � ������� �����
// �� ����������� ������. ���� ������ ������� �� ����� � ������� ������, �� �������
// ����������� �� ��������� ������.
//  message: ��������� �� ������ �������� � ����������� ������. 0x00 - ������� ����� ������.
void PCF8814::PrintWide(char * message)
{
  while (*message) PutcWide(*message++);  // ����� ������ ��������� �����
}

//******************************************************************************
// ����� ������ �������� �� LCD-����� NOKIA 1100 � ������� ����� �� ����������� ������.
// ���� ������ ������� �� ����� � ������� ������, �� ������� ����������� �� ��������� ������.
//  message: ��������� �� ������ �������� � ����������� ������. 0x00 - ������� ����� ������.
void PCF8814::PrintF(char * message)
{
  byte data;
  while (data=pgm_read_byte(message), data)
  { 
    Putc(data);
    message++;
  }
}

//******************************************************************************
// ������������� ������ � ����������� ���������. ������ ���������� � ������� 
// ����� ����. �� ����������� 16 ���������, �� ��������� - 8
//  x: 0..15
//  y: 0..7    
void PCF8814::GotoXY(byte x,byte y)
{
  x=x*6;	// ��������� �� ���������� � ����������� � ����������� � ��������

  lcd_xcurr=x;
  lcd_ycurr=y;

  SendByte(CMD_LCD_MODE,(0xB0|(y&0x0F)));      // ��������� ������ �� Y: 0100 yyyy         
  SendByte(CMD_LCD_MODE,(0x00|(x&0x0F)));      // ��������� ������ �� X: 0000 xxxx - ���� (x3 x2 x1 x0)
  SendByte(CMD_LCD_MODE,(0x10|((x>>4)&0x07))); // ��������� ������ �� X: 0010 0xxx - ���� (x6 x5 x4)

}

//******************************************************************************
// ������������� ������ � ��������. ������ ���������� � ������� 
// ����� ����. �� ����������� 96 ��������, �� ��������� - 65
//  x: 0..95
//  y: 0..64
void PCF8814::GotoXY_pix(byte x,byte y)
{
  lcd_xcurr=x;
  lcd_ycurr=y/8;

  SendByte(CMD_LCD_MODE,(0xB0|(lcd_ycurr&0x0F)));      // ��������� ������ �� Y: 0100 yyyy         
  SendByte(CMD_LCD_MODE,(0x00|(x&0x0F)));      // ��������� ������ �� X: 0000 xxxx - ���� (x3 x2 x1 x0)
  SendByte(CMD_LCD_MODE,(0x10|((x>>4)&0x07))); // ��������� ������ �� X: 0010 0xxx - ���� (x6 x5 x4)
}

//******************************************************************************
// ����� ����� �� LCD-�����
//  x: 0..95  ���������� �� ����������� (������ �� �������� ������ ����)
//	y: 0..64  ���������� �� ���������
//	pixel_mode: PIXEL_ON  - ��� ��������� ��������
//				PIXEL_OFF - ��� ���������� �������
//				PIXEL_INV - ��� �������� �������
void PCF8814::Pixel(byte x,byte y, byte pixel_mode)
{
  byte temp;

  GotoXY_pix(x,y);        
  temp=lcd_memory[lcd_xcurr][lcd_ycurr];

  switch(pixel_mode)
  {
  case PIXEL_ON:
    SetBit(temp, y%8);			// �������� ������
    break;
  case PIXEL_OFF:
    ClearBit(temp, y%8);		// ��������� ������
    break;
  case PIXEL_INV:
    InvBit(temp, y%8);			// ����������� ������
    break;
  }

  lcd_memory[lcd_xcurr][lcd_ycurr] = temp; // �������� ���� � ����������
  SendByte(DATA_LCD_MODE,temp); // �������� ���� � ����������
}

//******************************************************************************
// ����� ����� �� LCD-�����
//  x1, x2: 0..95  ���������� �� ����������� (������ �� �������� ������ ����)
//	y1, y2: 0..64  ���������� �� ���������
//	pixel_mode: PIXEL_ON  - ��� ��������� ��������
//				PIXEL_OFF - ��� ���������� �������
//				PIXEL_INV - ��� �������� �������
void PCF8814::Line (byte x1,byte y1, byte x2,byte y2, byte pixel_mode)
{
  int dy, dx;
  signed char addx = 1, addy = 1;
  signed int 	P, diff;

  byte i = 0;

  dx = abs((signed char)(x2 - x1));
  dy = abs((signed char)(y2 - y1));

  if(x1 > x2)	addx = -1;
  if(y1 > y2)	addy = -1;

  if(dx >= dy)
  {
    dy *= 2;
    P = dy - dx;

    diff = P - dx;

    for(; i<=dx; ++i)
    {
      Pixel(x1, y1, pixel_mode);

      if(P < 0)
      {
        P  += dy;
        x1 += addx;
      }
      else
      {
        P  += diff;
        x1 += addx;
        y1 += addy;
      }
    }
  }
  else
  {
    dx *= 2;
    P = dx - dy;
    diff = P - dy;

    for(; i<=dy; ++i)
    {
      Pixel(x1, y1, pixel_mode);

      if(P < 0)
      {
        P  += dx;
        y1 += addy;
      }
      else
      {
        P  += diff;
        x1 += addx;
        y1 += addy;
      }
    }
  }
}



//******************************************************************************
// ����� ���������� �� LCD-�����
//  x: 0..95  ���������� ������ ���������� (������ �� �������� ������ ����)
//	y: 0..64  ���������� �� ���������
//  radius:   ������ ����������
//  fill:		FILL_OFF  - ��� ������� ����������
//				FILL_ON	  - � ��������
//	pixel_mode: PIXEL_ON  - ��� ��������� ��������
//				PIXEL_OFF - ��� ���������� �������
//				PIXEL_INV - ��� �������� �������

void PCF8814::Circle(byte x, byte y, byte radius, byte fill, byte pixel_mode)
{
  signed char  a, b, P;

  a = 0;
  b = radius;
  P = 1 - radius;

  do
  {
    if(fill)
    {
      Line(x-a, y+b, x+a, y+b, pixel_mode);
      Line(x-a, y-b, x+a, y-b, pixel_mode);
      Line(x-b, y+a, x+b, y+a, pixel_mode);
      Line(x-b, y-a, x+b, y-a, pixel_mode);
    }
    else
    {
      Pixel(a+x, b+y, pixel_mode);
      Pixel(b+x, a+y, pixel_mode);
      Pixel(x-a, b+y, pixel_mode);
      Pixel(x-b, a+y, pixel_mode);
      Pixel(b+x, y-a, pixel_mode);
      Pixel(a+x, y-b, pixel_mode);
      Pixel(x-a, y-b, pixel_mode);
      Pixel(x-b, y-a, pixel_mode);
    }

    if(P < 0) P += 3 + 2 * a++;
    else P += 5 + 2 * (a++ - b--);
  } 
  while(a <= b);
}



//******************************************************************************
// ����� �������������� �� LCD-�����
//  x1, x2: 0..95  ���������� �� ����������� (������ �� �������� ������ ����)
//	y1, y2: 0..64  ���������� �� ���������
//	pixel_mode: PIXEL_ON  - ��� ��������� ��������
//				PIXEL_OFF - ��� ���������� �������
//				PIXEL_INV - ��� �������� �������
void PCF8814::Rect (byte x1, byte y1, byte x2, byte y2, byte fill, byte pixel_mode)
{
  if(fill)
  {			// � ��������
    byte  i, xmin, xmax, ymin, ymax;

    if(x1 < x2) { 
      xmin = x1; 
      xmax = x2; 
    }	// ���������� ����������� � ������������ ���������� �� X
    else { 
      xmin = x2; 
      xmax = x1; 
    }

    if(y1 < y2) { 
      ymin = y1; 
      ymax = y2; 
    }	// ���������� ����������� � ������������ ���������� �� Y
    else { 
      ymin = y2; 
      ymax = y1; 
    }

    for(; xmin <= xmax; ++xmin)
    {
      for(i=ymin; i<=ymax; ++i) Pixel(xmin, i, pixel_mode);
    }
  }
  else		// ��� �������
  {
    Line(x1, y1, x2, y1, pixel_mode);		// ������ ������� ��������������
    Line(x1, y2, x2, y2, pixel_mode);
    Line(x1, y1+1, x1, y2-1, pixel_mode);
    Line(x2, y1+1, x2, y2-1, pixel_mode);
  }
}

//******************************************************************************
// ����� �������� �� LCD-�����
//  x: 0..95  ���������� �������� ������ ���� �� ����������� (������ �� �������� ������ ���� ������)
//	y: 0..64  ���������� �������� ������ ���� �� ���������
//  picture: ��������� �� ������ � ����������� ��������� � ����������� ������, ������ 2 ����� ��������� ��������������
//			 ������ �������� �� ����������� � ��������� 
void PCF8814::Pict  (byte x, byte y, byte * picture)
{
  byte pict_width = pgm_read_byte(&picture[0]);  // ������ ������� � ��������  
  byte pict_height = pgm_read_byte(&picture[1]); // ������ ������� � ��������
  byte pict_height_bank=pict_height / 8+((pict_height%8)>0?1:0); // ������ ������� � ������
  byte y_pos_in_bank = y/8 + ((y%8)>0?1:0);		// ������� �� y � ������ (������� �� 8 ����.)

  int adr = 2; // ������ �������� ����� � ������� � ���������
  byte i;
  for (i=0; i< pict_height_bank; i++)
  { // ������ ��������� (�� ������)

    if (i<((LCD_Y_RES/8)+1)) // �� �������� �������� �� ��������� ������
    {
      //���������������� �� ����� ������
      lcd_xcurr=x;
      lcd_ycurr=y_pos_in_bank + i;

      SendByte(CMD_LCD_MODE,(0xB0|((y_pos_in_bank+i)&0x0F))); // ��������� ������ �� Y: 0100 yyyy         
      SendByte(CMD_LCD_MODE,(0x00|(x&0x0F)));      // ��������� ������ �� X: 0000 xxxx - ���� (x3 x2 x1 x0)
      SendByte(CMD_LCD_MODE,(0x10|((x>>4)&0x07))); // ��������� ������ �� X: 0010 0xxx - ���� (x6 x5 x4)

      //����� ������
      byte j;
      for ( j = 0; j < pict_width; j++ )
      {
        if ((x+j) < LCD_X_RES) SendByte(DATA_LCD_MODE,pgm_read_byte(&picture[adr])); // �� �������� �������� �� ��������� ������
        adr++;
      }
    }
  }
}
