

int Data[63];

void f_startTrame()
{
  int B_start = 0b0;
  //début de la trame commence par un 0
  return B_start;
  //0x0
}


void f_Arbitration()
{
  int B_id = 0b00000000001; //A (unique) identifier which also represents the message priority; le ID avec le plus petit nombre de bits définie le dominant NODE (priority)
  
  //RTR
  int B_rtr = 0; //Must be dominant (0) for data frames and recessive (1) for remote request frames

  return B_id + B_rtr;
  // 0x000000000010
}

void f_control()
{
  int B_IDE = 0b0; //Must be dominant (0) for base frame format with 11-bit identifiers
  int B_reservedBit = 0b0; //Reserved bit. Must be dominant (0), but accepted as either dominant or recessive.
  int N_dataLenght; // Number of bytes of data (0–8 bytes/0-64 bits)
  int B_dataLenght;

  for(int i=63; i>=63 ; i++)
  {
    if(Data[i] == 1)
    {
      i=N_dataLenght;
    }
  }
  B_dataLenght = int_to_bin(N_dataLenght);
  //0x0000

  return B_IDE + B_reservedBit + B_dataLenght;
}
void f_CyclicRedundancyCheck()
{

  int B_crcDelimiter = 0b1; //Transmitter sends recessive (1) and any receiver can assert a dominant (0)
}
void f_ACK()
{
  int B_crc = 0b111011101010011;// ***EXEMPLE CRC 15 bits Cyclic Redundancy Check
  int B_crcDelimiter = 0b1; //Must be recessive (1)
  int B_ackSlot = 0b0; //	Transmitter sends recessive (1) and any receiver can assert a dominant (0)
  int B_ackDelimiter = 0b1; //Must be recessive (1)

  return B_ackSlot + B_ackDelimiter;
  //0x01
}
void f_endTrame()
{
    int B_endofFrame = 0b1111111; // 7 x Must be recessive (1)
    int B_interFrame = 0b111; // 3 x Must be recessive (1)

    return B_endofFrame + B_interFrame;
    //0b1111111111
}

//trame complete exemple avec stuff bit
//0000010010100000100010000010011110111010100111011111111111

//trame complet sans stuff bit
//0000000101000000001000000011110111010100111011111111111



int int_to_bin(int k)
{
   char *bin;
   int tmp;

   bin = calloc(1, 1);
   while (k > 0)
   {
      bin = realloc(bin, strlen(bin) + 2);
      bin[strlen(bin) - 1] = (k % 2) + '0';
      bin[strlen(bin)] = '\0';
      k = k / 2;
   }
   tmp = atoi(bin);
   free(bin);
   return tmp;
}