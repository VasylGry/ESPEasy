//
//  _P137_Pages.ino
//
/*
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>

//********************************************************************************
// Control Screen
//********************************************************************************
void Plugin_137_StatScreen(Adafruit_ILI9341& tft, bool withTouch)
{

  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(80, 0);
  tft.setTextColor(ILI9341_GREEN);  
  tft.setTextSize(2);
  tft.println("Statistic");

  if(withTouch)
  {
    btnLight.draw(tft, btnLight.state);
    btnPump.draw(tft, btnPump.state);
    btnFan.draw(tft, btnFan.state);
    btnHeat.draw(tft, btnHeat.state);
    btnPage.draw(tft, false);
    doTouch = false;
  }
}
*/