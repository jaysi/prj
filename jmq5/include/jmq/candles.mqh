#ifndef JCANDLE_MQH
#define JCANDLE_MQH
/*
   enums
*/

#include "jbase.mqh"
 
enum jcandle_types {
   JCANDLE_TYPE_EMPTY,   
   JCANDLE_TYPE_INVAL
};

enum jcandle_fill {
   JCANDLE_FILL_EMPTY,
   JCANDLE_FILL_FULL,
   JCANDLE_FILL_ZERO,
   JCANDLE_FILL_INVAL
};

class JCandle {   
      
   public:
   int JCandle(string symbol_str = Symbol());
   int update(datetime date = 0);
   int enum_candles(datetime start, datetime end);
   string get_symbol();
   enum jcandle_types get_type();
   enum jcandle_fill get_fill();
   
   private:
   string symbol;
   double open, close, high, low, body, upshadow, lowshadow;
   enum jcandle_fill fill;
   double pair_open, pair_close, pair_high, pair_low;
   datetime enum_candle_ptr;
};

int JCandle::JCandle(string symbol_str){
   
   symbol = symbol_str;
   update();
   return JE_OK;
}

int JCandle::update(datetime date){
   struct MqlRates rates[];
   ArraySetAsSeries(rates, true);
   CopyRates(symbol, 0, date, 1, rates);
   open = rates[0].open;
   close = rates[0].close;
   high = rates[0].high;
   low = rates[0].low;
   NormalizeDouble(open, JDOUBLE_PRESC);
   NormalizeDouble(close, JDOUBLE_PRESC);
   NormalizeDouble(high, JDOUBLE_PRESC);
   NormalizeDouble(low, JDOUBLE_PRESC);   
   if(open < close) fill = JCANDLE_FILL_EMPTY;
   else if(open > close) fill = JCANDLE_FILL_FULL;
   else fill = JCANDLE_FILL_ZERO;
   return JE_OK;
}

string JCandle::get_symbol(void){
   return symbol;
}

enum jcandle_fill JCandle::get_fill(){
   return fill;
}

enum jcandle_types JCandle::get_type(){
   
}

#else

#endif