void modesTick() {
  button.tick();
  boolean changeFlag = false;
  if (button.isClick()) {
    mode++;
    if (mode > 1) mode = 0; changeFlag = true;
  }

  if (button.isHolded()) {
    mode = 0;
    changeFlag = true;
    lcd.noBacklight();
  }

  if (button.isDouble()) {
    lcd.backlight();
  }

  if (changeFlag) {
    if (mode == 0) {
      lcd.clear();
      loadClock();
      drawClock(hrs, mins, 0, 0, 1);
      drawData();
      DrawBL999();
    } else {
      lcd.clear();
      redrawPlot();
    }
  }
}

void redrawPlot() {
  lcd.clear();
  switch (mode) {
    case 1: DrawBME() ;
      break;
  }
}
