#include <M5Stack.h>
#include <M5StackUpdater.h>
#include "src/avatar.h"

Avatar *avatar;
TaskHandle_t taskHandle;
extern const unsigned char avatar_uma_bg[];

void breath(void *args)
{
  int c = 0;
  for(;;)
  {
    c = c + 1 % 100;
    float f = sin(c * 2 * PI / 100.0);
    avatar->setBreath(f);
    delay(33);
  }
}
void drawLoop(void *args)
{
  for(;;)
  {
    avatar->draw();
    delay(100);
  }
  
}

void saccade(void *args)
{
  for(;;)
  {
    float vertical = (float)rand()/(float)(RAND_MAX / 2) - 1;
    float horizontal = (float)rand()/(float)(RAND_MAX / 2) - 1;
    avatar->setGaze(vertical, horizontal);
    delay(500 + 100 * random(20));
  }
}

void blink(void *args)
{
  for(;;)
  {
      avatar->setEyeOpen(1);
      delay(2500 + 100 * random(20));
      avatar->setEyeOpen(0);
      delay(300 + 10 * random(20));
  }
}

void startAvatar()
{
  avatar = new Avatar();
  xTaskCreatePinnedToCore(
                    drawLoop,     /* Function to implement the task */
                    "drawLoop",   /* Name of the task */
                    2048,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    1,         /* Priority of the task */
                    &taskHandle,      /* Task handle. */
                    1);        /* Core where the task should run */
  xTaskCreatePinnedToCore(
                    saccade,     /* Function to implement the task */
                    "saccade",   /* Name of the task */
                    2048,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    3,         /* Priority of the task */
                    NULL,      /* Task handle. */
                    0);        /* Core where the task should run */
  xTaskCreatePinnedToCore(
                    breath,     /* Function to implement the task */
                    "breath",   /* Name of the task */
                    2048,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    2,         /* Priority of the task */
                    NULL,      /* Task handle. */
                    0);        /* Core where the task should run */
  xTaskCreatePinnedToCore(
                    blink,     /* Function to implement the task */
                    "blink",   /* Name of the task */
                    2048,      /* Stack size in words */
                    NULL,      /* Task input parameter */
                    0,         /* Priority of the task */
                    NULL,      /* Task handle. */
                    1);        /* Core where the task should run */  
}

void setup()
{
  M5.begin();
  Wire.begin();
  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }
  M5.Lcd.setBrightness(30);
  M5.Lcd.clear();
  M5.Lcd.setRotation(1);
  M5.Lcd.drawJpg(avatar_uma_bg, 2895);
  startAvatar(); // start drawing
}

void loop()
{
  // avatar's face updates in another thread
  // so no need to loop-by-loop rendering
  M5.update();
  if (M5.BtnA.wasPressed()) {
    avatar->setMouthOpen(1);
  } else if (M5.BtnA.wasReleased()) {
    avatar->setMouthOpen(0);
  }
  if (M5.BtnB.wasPressed()) {
    avatar->setEyeOpen(0);
  } else if (M5.BtnB.wasReleased()) {
    avatar->setEyeOpen(1);
  }
  if (M5.BtnC.wasPressed()) {
    vTaskSuspend(taskHandle);
    avatar->extendAction();
  } else if (M5.BtnC.wasReleased()) {
    vTaskResume(taskHandle);
  }
}
