#define rSlider A0
#define gSlider A1
#define bSlider A2

#define NUM_SAMPLES 16

byte server[] = {192, 168, 0, 100};

static int lastR = -1, lastG = -1, lastB = -1;
static int minR = 250, minG = 250, minB = 250;
static int maxR = 3000, maxG = 3000, maxB = 3000;
static long sumR = 0, sumG = 0, sumB = 0;
static int readingsR[NUM_SAMPLES] = {0}, readingsG[NUM_SAMPLES] = {0}, readingsB[NUM_SAMPLES] = {0};
static int indexR=0, indexG=0, indexB=0;

int getReading(int analogPin, long &sum, int *readings, int &index);
void requestChange(int r,int g,int b);
TCPClient client;

void setup() 
{
    pinMode(rSlider, INPUT);
    pinMode(gSlider, INPUT);
    pinMode(bSlider, INPUT);
    
    Serial.begin(115200);
    
    for (int i=0; i < NUM_SAMPLES; ++i) {
        lastR = getReading(rSlider, sumR, readingsR, indexR, minR, maxR);
        lastG = getReading(gSlider, sumG, readingsG, indexG, minG, maxG);
        lastB = getReading(bSlider, sumB, readingsB, indexB, minB, maxB);
    }
    RGB.control(true);
}

void loop() 
{
    static bool changed = false;
    static int count = 0;

    int newR = getReading(rSlider, sumR, readingsR, indexR, minR, maxR);
    if (newR != lastR) {
        changed = true;
        lastR = newR;
    }
    int newG = getReading(gSlider, sumG, readingsG, indexG, minG, maxG);
    if (newG != lastG) {
        changed = true;
        lastG = newG;
    }
    int newB = getReading(bSlider, sumB, readingsB, indexB, minB, maxB);
    if (newB != lastB) {
        changed = true;
        lastB = newB;
    }
    
    if (changed) {
            RGB.color(lastR, lastG, lastB);
    }
    if (client.connected()) {
        if (client.available() || ++count == 100) {
            client.stop();
            count = 0;
        }
    } else {
        if (changed) {
            requestChange(lastR, lastG, lastB);
            changed = false;
        }
    }
    
    delay(10);
}

int getReading(int analogPin, long &sum, int *readings, int &index, int &min, int &max)
{
    int reading = analogRead(analogPin);
    if (reading < min) {
        // offset a bit to keep it from being on just a little
        min = reading * 1.05;
    }
    if (reading > max) {
        max = reading * .95;
    }
    sum -= readings[index];
    sum += reading;
    readings[index++] = reading;
    if (index == NUM_SAMPLES) {
        index = 0;
    }
    return constrain(map(sum / NUM_SAMPLES,min,max,0,255),0,255);
}

void requestChange(int r,int g,int b)
{
    if (client.connect(server, 80)) {
        // yes, I know it should be POST
        client.print("GET /lights?red=");
        client.print(r);
        client.print("&green=");
        client.print(g);
        client.print("&blue=");
        client.print(b);
        client.print("&node=[00:13:a2:00:40:7b:6a:85]!");
        client.print(" HTTP/1.0\n\n");
    }
}
