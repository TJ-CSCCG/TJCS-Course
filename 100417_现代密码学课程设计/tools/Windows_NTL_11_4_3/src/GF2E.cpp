

#include <NTL/GF2E.h>


NTL_START_IMPL

NTL_TLS_GLOBAL_DECL(SmartPtr<GF2EInfoT>, GF2EInfo_stg)

NTL_CHEAP_THREAD_LOCAL
GF2EInfoT *GF2EInfo = 0; 


GF2EInfoT::GF2EInfoT(const GF2X& NewP)
{
   build(p, NewP);
   _card_exp = p.n;

   long sz = p.size;

// The following crossovers were set using the programs
// GF2EXKarCross.cpp, GF2EXModCross.cpp, GF2EXModCross.cpp,
// and GF2EXGCDCross.cpp.
// To use these programs, one has to remove the #if 0 guards
// in GF2EX.cpp on mul_disable_plain, BuildPlain, and DivRemPlain.

// There are three different configurations that are treated separately:
//   * with gf2x lib and with pclmul instruction available
//   * without gf2x lib but with pclmul
//   * without gf2X lib and without pclmul
// It is possible that one could be using gf2x lib on a platform without
// pclmul, in which case the crossovers used here are not optimal.  It is also
// possible that one could be using gf2x lib with pclmul, but compile NTL with
// NATIVE=off, so that NTL assumes there is no pclmul.  Again, this will lead
// to crossovers that are not optimal.

// The crossovers were calculated based on a Skylake Xeon processor:
// Intel(R) Xeon(R) Gold 6132 CPU @ 2.60GHz.


#if (defined(NTL_GF2X_LIB) && defined(NTL_HAVE_PCLMUL))

   //========== KarCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         KarCross = 3;
      else
         KarCross = 4;
   }
   else if (sz <= 6) KarCross = 8;
   else if (sz <= 9) KarCross = 4;
   else              KarCross = 2;



   //========== ModCross ==========


   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         ModCross = 15;
      else
         ModCross = 20;
   }
   else if (sz <=  9) ModCross =  60;
   else if (sz <= 18) ModCross =  25;
   else               ModCross =  15;


   //========== DivCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         DivCross =  50;
      else
         DivCross =  75;
   }
   else if (sz <=  2) DivCross = 100;
   else if (sz <=  3) DivCross = 150;
   else if (sz <=  4) DivCross = 200;
   else if (sz <=  6) DivCross = 250;
   else if (sz <=  9) DivCross = 225;
   else if (sz <= 15) DivCross = 125;
   else if (sz < 125) DivCross = 100;
   else               DivCross =  75;

   //========== GCDCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         GCDCross = 225;
      else
         GCDCross = 225;
   }
   else if (sz <=  2) GCDCross =  450;
   else if (sz <=  4) GCDCross =  600;
   else if (sz <  12) GCDCross = 1150;
   else               GCDCross =  600;


#elif (defined(NTL_HAVE_PCLMUL))

   //========== KarCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         KarCross = 5;
      else
         KarCross = 8;
   }
   else if (sz <= 5) KarCross = 8;
   else if (sz <= 9) KarCross = 4;
   else              KarCross = 2;



   //========== ModCross ==========


   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         ModCross = 30;
      else
         ModCross = 45;
   }
   else if (sz <=  2) ModCross = 110;
   else if (sz <=  3) ModCross = 105;
   else if (sz <=  4) ModCross =  65;
   else if (sz <=  5) ModCross =  60;
   else if (sz <=  6) ModCross =  55;
   else if (sz <=  8) ModCross =  50;
   else if (sz <= 12) ModCross =  30;
   else if (sz <= 18) ModCross =  25;
   else               ModCross =  15;



   //========== DivCross ==========


   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         DivCross =  75;
      else
         DivCross = 125;
   }
   else if (sz <=  2) DivCross = 450;
   else if (sz <=  3) DivCross = 425;
   else if (sz <=  4) DivCross = 375;
   else if (sz <=  6) DivCross = 250;
   else if (sz <=  8) DivCross = 225;
   else if (sz <= 16) DivCross = 125;
   else if (sz <= 45) DivCross = 100;
   else               DivCross =  75;


   //========== GCDCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         GCDCross = 225;
      else
         GCDCross = 225;
   }
   else if (sz < 12) GCDCross = 1150;
   else              GCDCross =  850;

#else

   //========== KarCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         KarCross = 4;
      else
         KarCross = 12;
   }
   else if (sz <= 3) KarCross = 4;
   else              KarCross = 2;



   //========== ModCross ==========


   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         ModCross = 45;
      else
         ModCross = 65;
   }
   else if (sz <=  2) ModCross =  25;
   else               ModCross =  15;


   //========== DivCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         DivCross = 175;
      else
         DivCross = 250;
   }
   else if (sz <=  4) DivCross = 100;
   else               DivCross =  75;

   //========== GCDCross ==========

   if (sz <= 1) {
      if (deg(p) <= NTL_BITS_PER_LONG/2)
         GCDCross = 225;
      else
         GCDCross = 850;
   }
   else if (sz <  8) GCDCross =  850;
   else if (sz < 12) GCDCross =  600;
   else              GCDCross =  450;


#endif

}


const ZZ& GF2E::cardinality()
{
   if (!GF2EInfo) LogicError("GF2E::cardinality: undefined modulus");

   do { // NOTE: thread safe lazy init
      Lazy<ZZ>::Builder builder(GF2EInfo->_card);
      if (!builder()) break;
      UniquePtr<ZZ> p;
      p.make();
      power(*p, 2, GF2EInfo->_card_exp);
      builder.move(p);
   } while (0);

   return *GF2EInfo->_card;
}







void GF2E::init(const GF2X& p)
{
   GF2EContext c(p);
   c.restore();
}


void GF2EContext::save()
{
   NTL_TLS_GLOBAL_ACCESS(GF2EInfo_stg);
   ptr = GF2EInfo_stg;
}

void GF2EContext::restore() const
{
   NTL_TLS_GLOBAL_ACCESS(GF2EInfo_stg);
   GF2EInfo_stg = ptr;
   GF2EInfo = GF2EInfo_stg.get();
}



GF2EBak::~GF2EBak()
{
   if (MustRestore) c.restore();
}

void GF2EBak::save()
{
   c.save();
   MustRestore = true;
}


void GF2EBak::restore()
{
   c.restore();
   MustRestore = false;
}



const GF2E& GF2E::zero()
{
   static const GF2E z(INIT_NO_ALLOC); // GLOBAL (assumes C++11 thread-safe init)
   return z;
}



istream& operator>>(istream& s, GF2E& x)
{
   GF2X y;

   NTL_INPUT_CHECK_RET(s, s >> y);
   conv(x, y);

   return s;
}

void div(GF2E& x, const GF2E& a, const GF2E& b)
{
   GF2E t;

   inv(t, b);
   mul(x, a, t);
}

void div(GF2E& x, GF2 a, const GF2E& b)
{
   inv(x, b);
   mul(x, x, a);
}

void div(GF2E& x, long a, const GF2E& b)
{
   inv(x, b);
   mul(x, x, a);
}


void inv(GF2E& x, const GF2E& a)
{
   InvMod(x._GF2E__rep, a._GF2E__rep, GF2E::modulus());
}


NTL_END_IMPL
