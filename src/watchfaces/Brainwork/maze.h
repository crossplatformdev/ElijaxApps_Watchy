void WatchyBrain::drawMaze(int variant, float batt) {
  const uint16_t white = UiSDK::getWatchfaceBg(BASE_POLARITY);
  const uint16_t black = UiSDK::getWatchfaceFg(BASE_POLARITY);
  const unsigned char *images [10] = {img0,img1,img2,img3,img4,img5,img6,img7,img8,img9};
            
      //drawbg
      display.fillScreen((variant == 0) ? black : white);
      display.drawBitmap(0, 0, background, 200, 200, (variant == 0) ? white : black);

      //draw time
      display.fillRect(29,29,66,66, (variant == 0) ? black : white);
      display.fillRect(29,107,66,66, (variant == 0) ? black : white);  
      display.fillRect(107,29,66,66, (variant == 0) ? black : white);  
      display.fillRect(107,107,66,66, (variant == 0) ? black : white); 
                  
      display.drawBitmap(29, 29, images[currentTime.Hour/10], 66, 66, (variant == 0) ? white : black);
      display.drawBitmap(107, 29, images[currentTime.Hour%10], 66, 66, (variant == 0) ? white : black);
                
      display.drawBitmap(29, 107, images[currentTime.Minute/10], 66, 66, (variant == 0) ? white : black);
      display.drawBitmap(107, 107, images[currentTime.Minute%10], 66, 66, (variant == 0) ? white : black);
     
      // draw battery
      if (batt > 0) {
       display.fillRect(0,185,2,12*batt,(variant == 0) ? black : white);
      }   
}
