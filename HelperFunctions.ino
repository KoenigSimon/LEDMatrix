void fillPanel(uint8_t p, const long *dat)
{
  for(int i = (int)(p * 64); i < (int)((p*64) + 64); i++)
  {
    setLedFromImg(dat, i, i);
  }  
}

void fillPanelZigZag(uint8_t p, const long *dat)
{
  unsigned int pStart = p * 64;
  for(int i = pStart; i < (int)(pStart + 64); i++)
  {
    if( (int)((i % 64) / 8) % 2 == 0 )
    {
      //fill forwards
      setLedFromImg(dat, i, i);
      Serial.print(i);
    }
    else
    {
      //fill backwards
      int row = ((i % 64) / 8);
      int last = ((row + 1) * 8);
      setLedFromImg(dat, pStart + (last - (i % 8) - 1), i);
      Serial.print(pStart + last - (i % 8) - 1);
    }

    
    Serial.print("  ");
    if(i % 8 == 7) Serial.println();
      
  }  
}

void setLedFromImg(const long *dat, int ledIndex, int datIndex)
{
  if(brightnessFactor != 1.0)
  {
    leds[ledIndex].r = int(constrain((dat[datIndex % 64] >> 16) * brightnessFactor, 0, 255));
    leds[ledIndex].g = int(constrain(((dat[datIndex % 64] & 0x00ff00) >> 8) * brightnessFactor, 0, 255));
    leds[ledIndex].b = int(constrain(((dat[datIndex % 64] & 0x0000ff)) * brightnessFactor, 0, 255));
  }
  else
  {
    leds[ledIndex].r =  dat[datIndex % 64] >> 16;
    leds[ledIndex].g = (dat[datIndex % 64] & 0x00ff00) >> 8;
    leds[ledIndex].b = (dat[datIndex % 64] & 0x0000ff);
  }  
}

static void RotateMatrix(const long* matrix, int xy) 
{  
  transpose(matrix, xy, xy);
  memcpy(result, temp, sizeof(long)*(xy * xy));
  reverseRows(xy);
}

static void MirrorMatrix(const long* matrix, int xy)
{
  memcpy(result, matrix, sizeof(long)*(xy * xy));
  reverseRows(xy);
}

static void reverseRows(int xy){
  // Traverse each row of arr[][] 
  
    for (int i = 0; i < xy; i++) { 
  
        // Initialise start and end index 
        int start = 0; 
        int end = xy - 1; 
  
        // Till start < end, swap the element 
        // at start and end index 
        while (start < end) { 
  
            // Swap the element 
            swap(&result[(i * xy) + start],  
                 &result[(i * xy) + end]); 
  
            // Increment start and decrement 
            // end for next pair of swapping 
            start++; 
            end--; 
        } 
    } 
}

void transpose(const long *arr, int m, int n){
    for (int i = 0; i < m; ++i )
    {
       for (int j = 0; j < n; ++j )
       {
          // Index in the original matrix.
          int index1 = i*n+j;

          // Index in the transpose matrix.
          int index2 = j*m+i;

          temp[index2] = arr[index1];
       }
    }
}


void swap(long* a, long* b) 
{ 
    long temp = *a; 
    *a = *b; 
    *b = temp; 
} 


//rgb to hsv 
float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

float* rgb2hsv(float r, float g, float b, float* hsv) {
  float s = step(b, g);
  float px = mix(b, g, s);
  float py = mix(g, b, s);
  float pz = mix(-1.0, 0.0, s);
  float pw = mix(0.6666666, -0.3333333, s);
  s = step(px, r);
  float qx = mix(px, r, s);
  float qz = mix(pw, pz, s);
  float qw = mix(r, px, s);
  float d = qx - min(qw, py);
  hsv[0] = abs(qz + (qw - py) / (6.0 * d + 1e-10));
  hsv[1] = d / (qx + 1e-10);
  hsv[2] = qx;
  return hsv;
}
