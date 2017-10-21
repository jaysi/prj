#ifndef JCANDLE_MQH
#define JCANDLE_MQH
/*
   enums
*/

#include "jbase.mqh"

#define _deb	printf

enum jcandle_types {
   JCANDLE_TYPE_EMPTY,
   JCANDLE_TYPE_DOJI_THUMBSTONE,
   JCANDLE_TYPE_DOJI_STAR,
   JCANDLE_TYPE_DOJI_SPINNER,
   JCANDLE_TYPE_DOJI_DRAGONFLY,
   JCANDLE_TYPE_HAMMER,
   JCANDLE_TYPE_SHOOTING_STAR,
   JCANDLE_TYPE_HANGING_MAN,
   JCANDLE_TYPE_HARAMI,
   JCANDLE_TYPE_BULLISH_ENGULFING,
   JCANDLE_TYPE_BEARISH_ENGULFING,
   JCANDLE_TYPE_DARK_CLOUD_COVER,
   JCANDLE_TYPE_PENETRATOR,
   JCANDLE_TYPE_EVENING_STAR,
   JCANDLE_TYPE_MORNING_STAR,
   JCANDLE_TYPE_FALLING_THREE_METHODS,
   JCANDLE_TYPE_RISING_THREE_METHODS,
   JCANDLE_TYPE_THREE_BLACK_CROWS,
   JCANDLE_TYPE_THREE_WHITE_SOLDIERS,
   JCANDLE_TYPE_DOWNSIDE_TASUKI,
   JCANDLE_TYPE_THREE_LINE_STRIKE,   
   JCANDLE_TYPE_INVAL
};

enum jcandle_fill {
   JCANDLE_FILL_EMPTY,
   JCANDLE_FILL_FULL,
   JCANDLE_FILL_ZERO,
   JCANDLE_FILL_INVAL
};

#define JCANDLE_MAX_COMBO 3

enum jcandle_combo {
	JCANDLE_COMBO_SINGLE = 0,
	JCANDLE_COMBO_TWIN = 1,
	JCANDLE_COMBO_TRIPLE = 2,
	JCANDLE_COMBO_ALL = 3
};

class JCandle {
      
   public:
   int JCandle(string symbol_str = Symbol(), enum jbase_date_system date_system = JBASE_DATE_SYSTEM_JALALI);
   int update(datetime date = 0);
   int update_combo(int combo);
   int enum_candles(datetime start, datetime end);
   string get_symbol() {return symbol;}
   enum jcandle_types get_type();
   enum jcandle_fill get_fill() {return fill};
   
   private:
   string symbol;
   enum jbase_date_system date_sys;
   struct jcandle_properties{
   	datetime date;
   	double open, close, high, low, body, upedge, lowedge, upshadow, lowshadow;
   	enum jcandle_fill fill;
   };
   struct jcandle_properties prop[JCANDLE_MAX_COMBO];
   
};

int JCandle::JCandle(string symbol_str, enum jbase_date_system date_system){
   
   symbol = symbol_str;
   date_sys = date_system;   
   return JE_OK;
   
}

int JCandle::update(string date_needle, enum jcandle_combo combo){
   struct MqlRates rates[];
   enum jcandle_combo i;
   datetime start_date;
   struct MqlDateTime date_s;
   
   ArraySetAsSeries(rates, true);
   
   if(date_sys == JBASE_DATE_SYSTEM_JALALI){
   	
   }
   
   _deb("%s:%s%s", "#DEBUG", "updating, needle date = ", TimeToString(date_needle));

	if(!TimeToStruct(date_needle, &date_s)){
		printf("%s:%s\n", "#ERROR", "Bad date/time.");
		return -1;
	}
	
	date_s.day -= combo;
	
	//TODO: check to see if there is a valid candle at the date!

	start_date = StructToTime(date_s);
	
	_deb("%s:%s%s", "#DEBUG", "updating, start date = ", TimeToString(start_date));

   CopyRates(symbol, 0, start_date, combo + 1, rates);
   
   _deb("%s:%s", "#DEBUG", "Symbol is ", symbol);
   
	if(!TimeToStruct(start_date, &date_s)){
		printf("%s:%s\n", "#ERROR", "Bad date/time.");
		return -1;
	}   
   
   for(i = JCANDLE_COMBO_SINGLE; i <= combo; i++){
   
   	date_s.day++;
   
	   prop[i].date = StructToTime(date_s);	  	   
	   
	   prop[i].open = rates[i].open;
	   prop[i].close = rates[i].close;
	   prop[i].high = rates[i].high;
	   prop[i].low = rates[i].low;
	
	   if(D(prop[i].open) < D(prop[i].close)){
	      prop[i].fill = JCANDLE_FILL_EMPTY;
	      prop[i].lowedge = prop[i].open;
	      prop[i].upedge = prop[i].close;
	   } else if(D(prop[i].open) > D(prop[i].close)) {
	      prop[i].fill = JCANDLE_FILL_FULL;
	      prop[i].lowedge = prop[i].close;
	      prop[i].upedge = prop[i].open;
	   } else {
	      prop[i].fill = JCANDLE_FILL_ZERO;
	      prop[i].lowedge = prop[i].close;
	      prop[i].upedge = prop[i].open;
	   }
	   
	   if(D(prop[i].high) =< D(prop[i].upedge)) prop[i].upshadow = 0.0;
	   else prop[i].upshadow = D(prop[i].high) - D(prop[i].upedge);
	   
	   if(D(prop[i].low) >= D(prop[i].lowedge)) prop[i].lowshadow = 0.0;
	   else prop[i].lowshadow = D(prop[i].lowedge) - D(prop.low);
	   
	   _deb("%s:%s%i\nt%s=%s\nt%s=%s\nt%s=%s\n\t%s=%s\n\t", "#DEBUG", "dumping properties @",
	   		i,
	   		"date", TimeToString(prop[i].date),
	   		"open", prop[i].open,
	   		"close", prop[i].close,
	   		"high", prop[i].high,
	   		"low", prop[i].low,
	   		);
	   
	}
   
   return JE_OK;
}

enum jcandle_types JCandle::get_type(int combo){
	
}

#else

#endif