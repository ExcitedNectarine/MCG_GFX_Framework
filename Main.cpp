#include <iostream>
#include <cmath>
#include "MCG_GFX_Lib.h"
#include <SDL/SDL.h>
#include <glm/gtx/matrix_transform_2d.hpp>

/**
* Bresanham's line drawing algorithm implementation.
*
* Draws straight lines with no gaps. No anti-aliasing.
*
* Converted from psuedocode from this video (https://youtu.be/zytBpLlSHms?t=247) into C++ by me.
**/
void draw_line(const glm::ivec2& start, const glm::ivec2& end, const glm::ivec3& colour)
{
  // Find the difference between the vectors.
  float dx = static_cast<float>(end.x - start.x);
  float dy = static_cast<float>(end.y - start.y);

  // Find the sign of the difference.
  int sx = dx < 0 ? -1 : 1;
  int sy = dy < 0 ? -1 : 1;

  // If the line is longer across the X axis than it is along the Y axis.
  if (std::abs(dx) > std::abs(dy))
  {
    // Calculate the slope and pitch of the line.
    float slope = dy / dx;
    float pitch = start.y - slope * start.x;

    // For each pixel across the X axis.
    for (int x = start.x; x != end.x;)
    {
      // Draw the pixel, calculating the Y position of the pixel using the slope
      // and pitch.
      MCG::DrawPixel({ x, (slope * x + pitch) }, colour);

      // Increment X.
      x += sx;
    }
  }

  // If the line is longer across the Y axis than it is the X axis.
  else
  {
    // Calculate the slope and pitch of the line.
    float slope = dx / dy;
    float pitch = start.x - slope * start.y;

    // For each pixel along the Y axis.
    for (int y = start.y; y != end.y;)
    {
      // Draw the pixel, calculating the X position of the pixel using the slope
      // and pitch.
      MCG::DrawPixel({ (slope * y + pitch), y }, colour);

      // Increment Y.
      y += sy;
    }
  }

  MCG::DrawPixel(end, colour);
}

/**
* This function draws a triangle. It connects p1, p2 and p3 with lines of a certain
* colour.
**/
void draw_triangle(const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3, const glm::ivec3& colour)
{
  // Connect p1 and p2.
  draw_line(p1, p2, colour);

  // Then connect p2 to p3.
  draw_line(p2, p3, colour);

  // And then p3 back to p1.
  draw_line(p3, p1, colour);
}

/**
* This function draws a triangle. It connects p1, p2 and p3 with lines of a certain
* colour.
**/
void draw_triangle_rotated(const glm::ivec2& p1, const glm::ivec2& p2, const glm::ivec2& p3, const glm::ivec3& colour)
{
  // Increment the angle of rotation.
  static float angle = 0.0f;
  angle += 0.025f;
  if (angle >= 360) angle = 0.0f;

  // Create a rotation matrix based off of the angle of the rotation.
  glm::mat2 rotation(std::cos(angle), -std::sin(angle),
                     std::sin(angle), std::cos(angle));

  // Apply the rotation to each of the points in the triangle.
  glm::ivec2 rp1 = p1 * rotation;
  glm::ivec2 rp2 = p2 * rotation;
  glm::ivec2 rp3 = p3 * rotation;

  // Connect p1 and p2.
  draw_line(rp1, rp2, colour);

  // Then connect p2 to p3.
  draw_line(rp2, rp3, colour);

  // And then p3 back to p1.
  draw_line(rp3, rp1, colour);
}

/**
* This function draws a rectangle. It takes the dimensions and the position (topleft) of
* the square and a colour to draw. Only draws the outline of the shape.
**/
void draw_rectangle(const glm::ivec2& dimensions, const glm::ivec2& position, const glm::ivec3& colour)
{
  draw_line(position, { position.x + dimensions.x, position.y }, colour);
  draw_line({ position.x + dimensions.x, position.y }, position + dimensions, colour);
  draw_line(position + dimensions, { position.x, position.y + dimensions.y }, colour);
  draw_line({ position.x, position.y + dimensions.y }, position, colour);
}

/**
* This function draws a circle.
**/
void draw_circle(const glm::ivec2& centre, const int radius, const glm::ivec3& colour)
{
  // Define the point to go out from.
  glm::ivec2 point = centre;
  glm::ivec2 last = point, next;

  // Define the first point to draw a line from.
  last.x += std::cos(0.0) * radius;
  last.y += std::sin(0.0) * radius;

  // Go through each angle in a circle + 1 more to completely connect the lines.
  for (int angle = 0; angle < 361; angle++)
  {
    // Find the position of the point on the circumference of the circle at the current angle.
    point.x += std::cos(glm::radians(float(angle))) * radius;
    point.y += std::sin(glm::radians(float(angle))) * radius;

    // Set the point to next.
    next = point;

    // Draw a line between last and next points across the circumference..
    draw_line(last, next, colour);

    // Make the next point the last point.
    last = next;

    // Reset point back to the centre for the next loop.
    point = centre;
  }
}

void draw_fake_cube(const glm::ivec3& dimensions, const glm::ivec2& position, const glm::ivec3& colour)
{
  draw_rectangle({ dimensions.x, dimensions.y }, position, colour);
  draw_rectangle({ dimensions.x, dimensions.y }, { position.x + dimensions.z, position.y + dimensions.z }, colour);

  draw_line(position, { position.x + dimensions.z, position.y + dimensions.z }, colour);
  draw_line({ position.x + dimensions.x, position.y }, { position.x + dimensions.x + dimensions.z, position.y + dimensions.z }, colour);
  draw_line({ position.x + dimensions.x, position.y + dimensions.y }, { position.x + dimensions.x + dimensions.z, position.y + dimensions.y + dimensions.z }, colour);
  draw_line({ position.x, position.y + dimensions.y }, { position.x + dimensions.z, position.y + dimensions.y + dimensions.z }, colour);
}

float curve_lerp(float a, float b, float c, float t)
{
  return (std::pow((1.0f - t), 2.0f) * a) + ((c * 2.0f * t) * (1.0f - t)) + (std::pow(t, 2.0f) * b);
}

/**
* This function draws a curve using a start, end and one control point.
*/
void draw_curve(const glm::ivec2& start, const glm::ivec2& end, const glm::ivec2& control, const glm::ivec3& colour)
{
  // Kind of like the circle drawing function where we draw lines between points.
  // Store the last and next point.
  glm::ivec2 last = start, next;

  for (float t = 0.0f; t < 1.0f; t += 0.01f)
  {
    // Interpolate points in the line using the control point and the current "division" of the
    // line, t.
    next.x = curve_lerp(start.x, end.x, control.x, t);
    next.y = curve_lerp(start.y, end.y, control.y, t);

    // Draw the line between the points.
    draw_line(last, next, colour);

    // Set the last point to the next point for the next line to be drawn.
    last = next;
  }
}

/**
* This function draws a real spinning 3D cube. It uses full 3D coordinates and projection.
*
* Code for creating projection, model, and viewport taken and adapted from https://stackoverflow.com/questions/35261192/how-to-use-glmproject-to-get-the-coordinates-of-a-point-in-world-space
*/
void draw_real_cube(const glm::ivec3& dimensions, const glm::ivec3& position, const glm::ivec3& colour)
{
  // Number of vertices and edges
  const int VERTEX_NUM = 8;
  const int EDGES = 12;

  // Variables used for perspective projection.
  // Camera position in 3D space.
  glm::vec3 camera(0.0f, 0.0f, 0.0f);

  // The viewport
  glm::vec4 viewport(0.0f, 0.0f, 800.0f, 600.0f);

  // Create the transformation matrix for the cube.
  glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position));
  glm::mat4 projection = glm::frustum(-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 100.0f);

  // Array of projected vertices. These vertices are actually drawn.
  glm::ivec2 vertices_drawn[VERTEX_NUM];

  // Array of 3D vertices, unprojected. The 0s are there so that the cube rotates around its
  // origin properly. The position of the cube is added to the vertices later.
  glm::ivec3 vertices[VERTEX_NUM] = {
    { 0, 0, 0 },
    { dimensions.x, 0, 0 },
    { 0, 0, dimensions.z },
    { dimensions.x, 0, dimensions.z },
    { 0, dimensions.y, 0 },
    { dimensions.x, dimensions.y, 0 },
    { 0, dimensions.y, dimensions.z },
    { dimensions.x, dimensions.y, dimensions.z }
  };

  // Pairs of vertices which lines are drawn between to actually make up the cube.
  int vertex_pairs[EDGES][2] = {
    { 0, 1 },
    { 1, 3 },
    { 2, 3 },
    { 4, 0 },
    { 2, 0 },
    { 2, 6 },
    { 4, 6 },
    { 6, 7 },
    { 5, 7 },
    { 3, 7 },
    { 4, 5 },
    { 1, 5 }
  };

  // Angle of rotation.
  static float angle = 0.0f;
  angle += 0.05f;
  if (angle >= 360) angle = 0.0f;

  for (int i = 0; i < VERTEX_NUM; i++)
  {
    glm::ivec3 temp = vertices[i];
    glm::mat4 rotation = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec4 rotated = rotation * glm::vec4(temp.x, temp.y, temp.z, 1.0f);

    // Project the 3D vertex onto 2D for drawing.
    vertices_drawn[i] = glm::project(glm::vec3(rotated), model, projection, viewport);
  }

  for (int i = 0; i < EDGES; i++)
  {
    glm::ivec2 vertex1 = vertices_drawn[vertex_pairs[i][0]];
    glm::ivec2 vertex2 = vertices_drawn[vertex_pairs[i][1]];
    draw_line(vertex1, vertex2, colour);
  }
}

/**
* This function draws the sierpiernski fractal. A triangle with multiple triangles within it
* that goes on for a set number of generations. The triangle is constructed from the inside out.
**/
void draw_sierpiernski_triangle(const glm::ivec2& x, const glm::ivec2& y, const glm::ivec2& z, const int generation, const glm::ivec3& colour)
{
  // When the generation gets to 1, it is time to draw the triangle.
  if (generation == 1)
  {
    // Draw the triangle.
    draw_triangle(x, y, z, colour);
  }
  // If the generation is not 1, we need to go deeper before drawing.
  else
  {
    // Find the midpoints of the lines of the triangle.
    glm::vec2 p1((x.x + y.x) / 2, (x.y + y.y) / 2);
    glm::vec2 p2((y.x + z.x) / 2, (y.y + z.y) / 2);
    glm::vec2 p3((z.x + x.x) / 2, (z.y + x.y) / 2);

    // Go down the generations for the top triangle.
    draw_sierpiernski_triangle(x, p1, p3, generation - 1, colour);

    // Go down the generations for the right triangle.
    draw_sierpiernski_triangle(p1, y, p2, generation - 1, colour);

    // Go down the generations for the left triangle.
    draw_sierpiernski_triangle(p3, p2, z, generation - 1, colour);
  }
}

int main(int argc, char *argv[])
{
  // Constants
  const glm::ivec2 WINDOW_SIZE(800, 600);
  const glm::ivec3 BACKGROUND_COLOUR(0, 0, 0);
  const int MAX_SLIDES = 7;

  glm::ivec2 mouse_position;
  SDL_Event event;
  int slide = 0;

  // Create window.
  if (!MCG::Init(WINDOW_SIZE))
    return -1;

  // Start main loop.
  while (MCG::ProcessFrame())
  {
    // Event loop.
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT) MCG::Cleanup();
      else if (event.type == SDL_KEYDOWN)
      {
        // If the left key is pressed, go down a slide.
        if (event.key.keysym.sym == SDLK_LEFT) slide--;
        // If the right key is pressed, go up a slide.
        else if (event.key.keysym.sym == SDLK_RIGHT) slide++;

        // Loop round the slides.
        if (slide < 0) slide = MAX_SLIDES;
        else if (slide > MAX_SLIDES) slide = 0;
      }
    }

    // Get the mouse position.
    SDL_GetMouseState(&mouse_position.x, &mouse_position.y);

    // Draw background.
    MCG::SetBackground(BACKGROUND_COLOUR);

    // Use a switch to find the current slide and draw it.
    switch (slide)
    {
    case 0:
      draw_line(WINDOW_SIZE / 2, mouse_position, glm::ivec3(255, 255, 255));
      break;
    case 1:
      draw_rectangle(WINDOW_SIZE - 100, glm::ivec2(50, 50), glm::ivec3(255, 0, 0));
      break;
    case 2:
      draw_triangle_rotated(glm::ivec2(100, 100), { 100, 200 }, glm::ivec2(200, 200), glm::ivec3(0, 255, 0));
      break;
    case 3:
      draw_circle(WINDOW_SIZE / 2, 250, glm::ivec3(0, 0, 255));
      break;
    case 4:
      draw_curve(glm::ivec2(50, 50), WINDOW_SIZE - 50, mouse_position, glm::ivec3(255, 255, 0));
      break;
    case 5:
      draw_sierpiernski_triangle(glm::ivec2(WINDOW_SIZE.x / 2, 50), WINDOW_SIZE - 50, glm::ivec2(50, WINDOW_SIZE.y - 50), 6, glm::ivec3(255, 0, 255));
      break;
    case 6:
      draw_fake_cube(glm::ivec3(200, 200, 50), (WINDOW_SIZE / 2) - 100, glm::ivec3(0, 255, 255));
      break;
    case 7:
      draw_real_cube(glm::ivec3(250, 250, 250), glm::ivec3(0, -50, 300), glm::ivec3(255, 255, 255));
      break;
    }
  }

  return 0;
}