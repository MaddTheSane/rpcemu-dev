/*RPCemu v0.6 by Tom Walker
  Main loop
  Not just for Linux - works as a Win32 console app as well*/

#include <allegro.h>
#include <pthread.h>
#include "rpcemu.h"
#include "mem.h"
#include "sound.h"
#include "vidc20.h"
#include "gui.h"


int mousecapture=0;
float mips;
int updatemips=0;

static uint32_t mipscount;
float mipstotal;

void wakeupsoundthread()
{
}

static void domips(void)
{
        mips=(float)inscount/1000000.0f;
	mipscount += 1;
	if (mipscount > 10)
	  mipstotal += mips;
        inscount=0;
        updatemips=1;
}

void error(const char *format, ...)
{
   va_list ap;
   va_start(ap, format);
   fprintf(stderr, "RPCemu error: ");
   vfprintf(stderr, format, ap);
   fprintf(stderr,"\n");
   va_end(ap);
}

FILE *arclog;
void rpclog(const char *format, ...)
{
   char buf[256];
//   return;
if (!arclog) arclog=fopen("rpclog.txt","wt");
   va_list ap;
   va_start(ap, format);
   vsprintf(buf, format, ap);
   va_end(ap);
   fputs(buf,arclog);
}

int drawscre=0;

void vblupdate()
{
        if (infocus) drawscre++;
}

void updatewindowsize(uint32_t x, uint32_t y)
{
}

void releasemousecapture()
{
}

int quited=0;

#ifdef VIDC_THREAD
pthread_t thread;
pthread_cond_t vidccond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t vidcmutex = PTHREAD_MUTEX_INITIALIZER;

static void *vidcthreadrunner(void *threadid)
{
        if (pthread_mutex_lock(&vidcmutex)) fatal("Cannot lock mutex");
	while (!quited)
	{
                if (pthread_cond_wait(&vidccond, &vidcmutex)) fatal("pthread_cond_wait failed");
		if (!quited) vidcthread();
	}
        pthread_mutex_unlock(&vidcmutex);
	return NULL;
}
#endif



void vidcstartthread(void)
{
#ifdef VIDC_THREAD
    if (pthread_create(&thread,NULL,vidcthreadrunner,NULL)) fatal("Couldn't create vidc thread");
#endif
}

void vidcendthread(void)
{
#ifdef VIDC_THREAD
	quited=1;
        if (pthread_cond_signal(&vidccond)) fatal("Couldn't signal vidc thread");
	pthread_join(thread, NULL);
#endif
}

void vidcwakeupthread(void)
{
#ifdef VIDC_THREAD
        if (pthread_cond_signal(&vidccond)) fatal("Couldn't signal vidc thread");
#else
        vidcthread();
#endif
}

int vidctrymutex(void)
{
#ifdef VIDC_THREAD
    int ret = pthread_mutex_trylock(&vidcmutex);
    if (ret == EBUSY) return 0;
    if (ret) fatal("Getting vidc mutex failed");
#endif
    return 1;
}

void vidcreleasemutex(void)
{
#ifdef VIDC_THREAD
    if (pthread_mutex_unlock(&vidcmutex)) fatal("Releasing vidc mutex failed");
#endif
}

void close_button_handler(void)
{
  quited = TRUE;
}
END_OF_FUNCTION(close_button_handler)

int main (void) 
{ 
        infocus=1;
        allegro_init();

	LOCK_FUNCTION(close_button_handler);
	set_close_button_callback(close_button_handler);

        install_keyboard();
        install_timer();
        install_mouse();

        if (startrpcemu())
           return -1;

        install_int_ex(domips,MSEC_TO_TIMER(1000));
        install_int_ex(vblupdate,BPS_TO_TIMER(refresh));
        if (soundenabled) initsound();
        infocus=1;
        mousehackon=1;

        while (!quited)
        {
                if (infocus)
                        execrpcemu();
                        if (updatemips)
                        {                           
                                printf("MIPS: %f (AVG: %f) %i\n", mips, mipstotal / (mipscount - 10),mousehack);
                                updatemips=0;
                        }
                if ((key[KEY_LCONTROL] || key[KEY_RCONTROL]) && key[KEY_END]) entergui();
                if ((key[KEY_LCONTROL] || key[KEY_RCONTROL]) && key[KEY_END] && mousecapture)
                {
                        mousecapture=0;
                        updatemips=1;
                }
        }
        if (mousecapture)
        {
                mousecapture=0;
        }
        endrpcemu();
        return 0;
}

END_OF_MAIN();
