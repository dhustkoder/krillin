#include "render.h"
#include "krlnet.h"




int main(void)
{
	printf("Hello Krillin\n");
	
	render_init();
	krlbot_init();
	
	while (render_poll_events()) {
		
		krlbot_update();
		
		actors_update();
		
		render_clear();
		
		actors_render();
		
		render_flush();
	}
	
	krlbot_term();
	render_term();
	
	return 0;
}

