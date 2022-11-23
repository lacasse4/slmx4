#ifndef SEND_DEF
#define SEND_DEF

enum type{int_, string_, sine_, float_};


int sendosc(type t, void* val,const char *host);


#endif
