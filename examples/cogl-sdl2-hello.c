#include <cogl/cogl.h>
#include <glib.h>
#include <stdio.h>
#include <SDL.h>

/* This short example is just to demonstrate mixing SDL with Cogl as a
   simple way to get portable support for events */

typedef struct Data
{
  CoglPrimitive *triangle;
  CoglPipeline *pipeline;
  float center_x, center_y;
  CoglFramebuffer *fb;
  CoglBool quit;
  CoglBool redraw_queued;
} Data;

static CoglBool
redraw (Data *data)
{
  CoglFramebuffer *fb = data->fb;

  cogl_framebuffer_clear4f (fb, COGL_BUFFER_BIT_COLOR, 0, 0, 0, 1);

  cogl_framebuffer_push_matrix (fb);
  cogl_framebuffer_translate (fb, data->center_x, -data->center_y, 0.0f);

  cogl_framebuffer_draw_primitive (fb, data->pipeline, data->triangle);
  cogl_framebuffer_pop_matrix (fb);

  cogl_onscreen_swap_buffers (COGL_ONSCREEN (fb));

  return FALSE;
}

static void
handle_event (Data *data, SDL_Event *event)
{
  switch (event->type)
    {
    case SDL_WINDOWEVENT:
      switch (event->window.event)
        {
        case SDL_WINDOWEVENT_EXPOSED:
          data->redraw_queued = TRUE;
          break;

        case SDL_WINDOWEVENT_CLOSE:
          data->quit = TRUE;
          break;
        }
      break;

    case SDL_MOUSEMOTION:
      {
        int width =
          cogl_framebuffer_get_width (COGL_FRAMEBUFFER (data->fb));
        int height =
          cogl_framebuffer_get_height (COGL_FRAMEBUFFER (data->fb));

        data->center_x = event->motion.x * 2.0f / width - 1.0f;
        data->center_y = event->motion.y * 2.0f / height - 1.0f;

        data->redraw_queued = TRUE;
      }
      break;

    case SDL_QUIT:
      data->quit = TRUE;
      break;
    }
}

int
main (int argc, char **argv)
{
  CoglContext *ctx;
  CoglOnscreen *onscreen;
  CoglError *error = NULL;
  CoglVertexP2C4 triangle_vertices[] = {
    {0, 0.7, 0xff, 0x00, 0x00, 0x80},
    {-0.7, -0.7, 0x00, 0xff, 0x00, 0xff},
    {0.7, -0.7, 0x00, 0x00, 0xff, 0xff}
  };
  Data data;
  SDL_Event event;

  ctx = cogl_sdl_context_new (SDL_USEREVENT, &error);
  if (!ctx)
    {
      fprintf (stderr, "Failed to create context: %s\n", error->message);
      return 1;
    }

  onscreen = cogl_onscreen_new (ctx, 800, 600);
  data.fb = COGL_FRAMEBUFFER (onscreen);

  data.center_x = 0.0f;
  data.center_y = 0.0f;
  data.quit = FALSE;

  cogl_onscreen_show (onscreen);

  data.triangle = cogl_primitive_new_p2c4 (ctx, COGL_VERTICES_MODE_TRIANGLES,
                                           3, triangle_vertices);
  data.pipeline = cogl_pipeline_new (ctx);

  data.redraw_queued = TRUE;
  while (!data.quit)
    {
      while (!data.quit)
        {
          if (!SDL_PollEvent (&event))
            {
              if (data.redraw_queued)
                break;

              cogl_sdl_idle (ctx);
              if (!SDL_WaitEvent (&event))
                {
                  fprintf (stderr, "Error waiting for SDL events");
                  return 1;
                }
            }

          handle_event (&data, &event);
          cogl_sdl_handle_event (ctx, &event);
        }

      data.redraw_queued = redraw (&data);
    }

  cogl_object_unref (ctx);

  return 0;
}
