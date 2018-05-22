/***************************************************************************
    Cannonball Main Entry Point.
    
    Copyright Chris White.
    See license.txt for more details.
***************************************************************************/

// Error reporting
#include <iostream>
#include <string.h>

#include "input.hpp"

#include "video.hpp"

#include "romloader.hpp"
#include "trackloader.hpp"
#include "stdint.hpp"
#include "main.hpp"
#include "setup.hpp"
#include "engine/outrun.hpp"
#include "frontend/config.hpp"
#include "frontend/menu.hpp"

#include "cannonboard/interface.hpp"
#include "engine/oinputs.hpp"
#include "engine/ooutputs.hpp"
#include "engine/omusic.hpp"

// Direct X Haptic Support.
// Fine to include on non-windows builds as dummy functions used.
#include "ffeedback.hpp"

// Initialize Shared Variables
using namespace cannonball;

int    cannonball::state       = STATE_BOOT;
double cannonball::frame_ms    = 0;
int    cannonball::frame       = 0;
bool   cannonball::tick_frame  = true;
int    cannonball::fps_counter = 0;

#ifdef COMPILE_SOUND_CODE
Audio cannonball::audio;
#endif

Menu* menu;
Interface cannonboard;

static void retro_unload_game_internal(void)
{
#ifdef COMPILE_SOUND_CODE
    audio.stop_audio();
#endif
    input.close();
    forcefeedback::close();
    delete menu;
}

static void process_events(void)
{
#if 0
    SDL_Event event;

    // Grab all events from the queue.
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:
                // Handle key presses.
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    state = STATE_QUIT;
                else
                    input.handle_key_down(&event.key.keysym);
                break;

            case SDL_KEYUP:
                input.handle_key_up(&event.key.keysym);
                break;

            case SDL_JOYAXISMOTION:
                input.handle_joy_axis(&event.jaxis);
                break;

            case SDL_QUIT:
                // Handle quit requests (like Ctrl-c).
                state = STATE_QUIT;
                break;
        }
    }
#endif
}

// Pause Engine
bool pause_engine;

static void config_init(void)
{
    // ------------------------------------------------------------------------
    // Menu Settings
    // ------------------------------------------------------------------------

    config.menu.enabled           = 1;
    config.menu.road_scroll_speed = 50;

    // ------------------------------------------------------------------------
    // Video Settings
    // ------------------------------------------------------------------------
   
    config.video.mode       = 0; // Video Mode: Default is Windowed 
    config.video.scale      = 2; // Video Scale: Default is 2x    
    config.video.scanlines  = 0; // Scanlines
    config.video.fps        = 2; // Default is 60 fps
    config.video.fps_count  = 0; // FPS Counter
    config.video.widescreen = 1; // Enable Widescreen Mode
    config.video.hires      = 0; // Hi-Resolution Mode
    config.video.filtering  = 0; // Open GL Filtering Mode
          
    config.set_fps(config.video.fps);

    // ------------------------------------------------------------------------
    // Sound Settings
    // ------------------------------------------------------------------------
    config.sound.enabled     = 0;
    config.sound.advertise   = 1;
    config.sound.preview     = 1;
    config.sound.fix_samples = 1;

#if 0
    // Custom Music
    for (int i = 0; i < 4; i++)
    {
        std::string xmltag = "sound.custom_music.track";
        xmltag += Utils::to_string(i+1);  

        config.sound.custom_music[i].enabled = pt_config.get(xmltag + ".<xmlattr>.enabled", 0);
        config.sound.custom_music[i].title   = pt_config.get(xmltag + ".title", "TRACK " +Utils::to_string(i+1));
        config.sound.custom_music[i].filename= pt_config.get(xmltag + ".filename", "track"+Utils::to_string(i+1)+".wav");
    }
#endif

#ifdef CANNONBOARD
    // ------------------------------------------------------------------------
    // CannonBoard Settings
    // ------------------------------------------------------------------------
    cannonboard.enabled = 1;
    cannonboard.port    = "COM6";
    cannonboard.baud    = 57600;
    cannonboard.debug   = 0;
    cannonboard.cabinet = 0;
#endif

    // ------------------------------------------------------------------------
    // Controls
    // ------------------------------------------------------------------------
    config.controls.gear          = 0;
    config.controls.steer_speed   = 3;
    config.controls.pedal_speed   = 4;
    config.controls.keyconfig[0]  = 273; /* up */
    config.controls.keyconfig[1]  = 274; /* down */
    config.controls.keyconfig[2]  = 276; /* left */
    config.controls.keyconfig[3]  = 275; /* right */
    config.controls.keyconfig[4]  = 122; /* accelerate */
    config.controls.keyconfig[5]  = 120; /* brake */
    config.controls.keyconfig[6]  = 32; /* gear1 */
    config.controls.keyconfig[7]  = 32; /* gear2 */
    config.controls.keyconfig[8]  = 49; /* start */
    config.controls.keyconfig[9]  = 53; /* coin */
    config.controls.keyconfig[10] = 286; /* menu */
    config.controls.keyconfig[11] = 304; /* view */
    config.controls.padconfig[0]  = 0; /* accelerate */
    config.controls.padconfig[1]  = 1; /* brake */
    config.controls.padconfig[2]  = 2; /* gear1 */
    config.controls.padconfig[3]  = 2; /* gear2 */
    config.controls.padconfig[4]  = 3; /* start */
    config.controls.padconfig[5]  = 4; /* coin */
    config.controls.padconfig[6]  = 5; /* padconfig menu */
    config.controls.padconfig[7]  = 6; /* padconfig view */
    config.controls.analog        = 0;
    config.controls.pad_id        = 0; /* pad_id */
    config.controls.axis[0]       = 0; /* wheel */
    config.controls.axis[1]       = 2; /* accelerate */
    config.controls.axis[2]       = 3; /* brake */
    config.controls.asettings[0]  = 75; /* wheel zone */
    config.controls.asettings[1]  = 0; /* wheel dead */
    config.controls.asettings[2]  = 0; /* pedals dead */
    
    config.controls.haptic        = 0;
    config.controls.max_force     = 9000;
    config.controls.min_force     = 8500;
    config.controls.force_duration= 20;

    // ------------------------------------------------------------------------
    // Engine Settings
    // ------------------------------------------------------------------------

    config.engine.dip_time      = 0;
    config.engine.dip_traffic   = 1;
    
    config.engine.freeze_timer    = config.engine.dip_time == 4;
    config.engine.disable_traffic = config.engine.dip_traffic == 4;
    config.engine.dip_time    &= 3;
    config.engine.dip_traffic &= 3;

    config.engine.freeplay      = 1;
    config.engine.jap           = 0; /* japanese tracks */
    config.engine.prototype     = 0;
    
    // Additional Level Objects
    config.engine.level_objects   = 1;
    config.engine.randomgen       = 1;
    config.engine.fix_bugs_backup = 
    config.engine.fix_bugs        = 1;
    config.engine.fix_timer       = 0;
    config.engine.layout_debug    = 0;
    config.engine.new_attract     = 1;

    // ------------------------------------------------------------------------
    // Time Trial Mode
    // ------------------------------------------------------------------------

    config.ttrial.laps    = 5;
    config.ttrial.traffic = 3;

    config.cont_traffic   = 3;

    config.fps     = 60;
}

static void retro_run_internal(void)
{
    frame++;

    // Get CannonBoard Packet Data
    Packet* packet = NULL;
    
#ifdef CANNONBOARD
    if (config.cannonboard.enabled)
       packet      = cannonboard.get_packet();
#endif

    // Non standard FPS.
    // Determine whether to tick the current frame.
    if (config.fps != 30)
    {
        if (config.fps == 60)
            tick_frame = frame & 1;
        else if (config.fps == 120)
            tick_frame = (frame & 3) == 1;
    }

    process_events();

    if (tick_frame)
        oinputs.tick(packet); // Do Controls
    oinputs.do_gear();        // Digital Gear

    switch (state)
    {
        case STATE_GAME:
        {
            if (input.has_pressed(Input::TIMER))
                outrun.freeze_timer = !outrun.freeze_timer;

            if (input.has_pressed(Input::PAUSE))
                pause_engine = !pause_engine;

            if (input.has_pressed(Input::MENU))
                state = STATE_INIT_MENU;

            if (!pause_engine || input.has_pressed(Input::STEP))
            {
                outrun.tick(packet, tick_frame);
                input.frame_done(); // Denote keys read

                #ifdef COMPILE_SOUND_CODE
                // Tick audio program code
                osoundint.tick();
                // Tick Audio
                audio.tick();
                #endif
            }
            else
            {                
                input.frame_done(); // Denote keys read
            }
        }
        break;

        case STATE_INIT_GAME:
            if (config.engine.jap && !roms.load_japanese_roms())
            {
                state = STATE_QUIT;
            }
            else
            {
                pause_engine = false;
                outrun.init();
                state = STATE_GAME;
            }
            break;

        case STATE_MENU:
        {
            menu->tick(packet);
            input.frame_done();
            #ifdef COMPILE_SOUND_CODE
            // Tick audio program code
            osoundint.tick();
            // Tick Audio
            audio.tick();
            #endif
        }
        break;

        case STATE_INIT_MENU:
            oinputs.init();
            outrun.outputs->init();
            menu->init();
            state = STATE_MENU;
            break;
    }

#ifdef CANNONBOARD
    // Write CannonBoard Outputs
    if (config.cannonboard.enabled)
        cannonboard.write(outrun.outputs->dig_out, outrun.outputs->hw_motor_control);
#endif

    // Draw Video
    video.draw_frame();  
}

static bool retro_load_game_internal(void)
{
    bool loaded = roms.load_revb_roms();

    if (!loaded)
       return false;

    config_init();

    // Load fixed PCM ROM based on config
    if (config.sound.fix_samples)
       roms.load_pcm_rom(true);

    // Load patched widescreen tilemaps
    if (!omusic.load_widescreen_map())
    {
       fprintf(stderr, "Unable to load widescreen tilemaps\n");
       return false;
    }

    if (!video.init(&roms, &config.video))
       return false;

    menu = new Menu(&cannonboard);

#ifdef COMPILE_SOUND_CODE
    audio.init();
#endif
    state = config.menu.enabled ? STATE_INIT_MENU : STATE_INIT_GAME;

    // Initialize controls
    input.init(config.controls.pad_id,
          config.controls.keyconfig, config.controls.padconfig, 
          config.controls.analog,    config.controls.axis, config.controls.asettings);

    if (config.controls.haptic) 
       config.controls.haptic = forcefeedback::init(config.controls.max_force, config.controls.min_force, config.controls.force_duration);

#ifdef CANNONBOARD
    // Initialize CannonBoard (For use in original cabinets)
    if (config.cannonboard.enabled)
    {
       cannonboard.init(config.cannonboard.port, config.cannonboard.baud);
       cannonboard.start();
    }
#endif

    // Populate menus
    menu->populate();
}

#if 0
int main(int argc, char* argv[])
{
    menu = new Menu(&cannonboard);

    bool loaded = false;

    // Load LayOut File
    if (argc == 3 && strcmp(argv[1], "-file") == 0)
    {
        if (trackloader.set_layout_track(argv[2]))
            loaded = roms.load_revb_roms(); 
    }
    // Load Roms Only
    else
    {
        loaded = roms.load_revb_roms();
    }

    //trackloader.set_layout_track("d:/temp.bin");
    //loaded = roms.load_revb_roms();

    if (loaded)
    {
        // Load XML Config
        config.load(FILENAME_CONFIG);

        // Load fixed PCM ROM based on config
        if (config.sound.fix_samples)
            roms.load_pcm_rom(true);

        // Load patched widescreen tilemaps
        if (!omusic.load_widescreen_map())
            std::cout << "Unable to load widescreen tilemaps" << std::endl;

        if (!video.init(&roms, &config.video))
            retro_unload_game_internal();

#ifdef COMPILE_SOUND_CODE
        audio.init();
#endif
        state = config.menu.enabled ? STATE_INIT_MENU : STATE_INIT_GAME;

        // Initialize controls
        input.init(config.controls.pad_id,
                   config.controls.keyconfig, config.controls.padconfig, 
                   config.controls.analog,    config.controls.axis, config.controls.asettings);

        if (config.controls.haptic) 
            config.controls.haptic = forcefeedback::init(config.controls.max_force, config.controls.min_force, config.controls.force_duration);
        
        // Initialize CannonBoard (For use in original cabinets)
        if (config.cannonboard.enabled)
        {
            cannonboard.init(config.cannonboard.port, config.cannonboard.baud);
            cannonboard.start();
        }

        // Populate menus
        menu->populate();
        while (state != STATE_QUIT) // Loop until we quit the app
           retro_run_internal();
    }

    retro_unload_game_internal();
    return 0;
}
#endif
