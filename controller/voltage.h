int low_voltage_flag = 1;
unsigned long vol_measure_time = 0;
float voltage_reading = 0.0;
void voltageInit()
{
  analogReference(INTERNAL);
}
void voltageMeasure() //测量电压
{
  if (millis() - vol_measure_time > 100) //每1000毫秒测量一次
  {
    vol_measure_time = millis();
    double voltage = (analogRead(VOL_MEASURE_PIN) * 1.1 / 1024) * ((10 + 1.5) / 1.5); //读取电压值
    voltage_reading = (float) voltage;
    if (voltage > 7.2)                                                                //电池供电
    {
      if (low_voltage_flag == 1)
      {
        digitalWrite(STBY_PIN,HIGH);
      }
      low_voltage_flag = 0; //满电压标志
    }
    else
    {
      if (voltage < 7.0) //The battery is low in power and needs to be charged.
      {
        //mode = STOP;
        digitalWrite(STBY_PIN,LOW);
      }
      //待机,停止,自动启动, 启动模式
      low_voltage_flag = 1; //低电压标志
    }
  }
}
