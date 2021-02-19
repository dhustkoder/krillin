#include "render.h"
#include "krlnet.h"

#include "krlss.h"
KRL_STATIC_STRING_DEFINE(test, 128);



float frame_delta;


int main(void)
{
	#if 0
	krlss_test_t str;
	
	krlss_assign_cstr(&str, "Hello World xxxxxxxxxxxxxxxxxxxxxxxxxxx!\n");
	
	printf("%s", str.data);
	
	#endif
	
	
	
	
	#if 1
	printf("Hello Krillin\n");
	
	
	render_init();

	actors_init();
	krlnet_init();
	krlbot_init();
	
	
	timer_t start = get_timer();
	while (render_poll_events()) {
		krlbot_update();

		actors_update();
		
		render_clear();
		
		actors_render();
		
		render_flush();
		
		frame_delta = (get_timer() - start) / 1000.0f;
		start = get_timer();
	}
	
	krlbot_term();
	krlnet_term();
	actors_term();

	render_term();
	#endif
	
	return 0;
}

