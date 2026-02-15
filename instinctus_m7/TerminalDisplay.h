#pragma once

#include "Arduino_GigaDisplay_GFX.h"
#include "Fonts/FreeMono12pt7b.h"

/**
 * Real-time event terminal display with flicker-free rendering.
 * Displays events in a scrolling terminal format. When screen fills,
 * it clears and starts fresh to keep text readable.
 */
class TerminalDisplay {
public:
    // Display configuration
    static const int MAX_LINES = 20;           // Lines that fit on screen in landscape
    static const int MAX_LINE_LENGTH = 66;     // Characters per line (800px / 12px)
    static const int CHAR_WIDTH = 12;          // FreeMono12pt character width
    static const int CHAR_HEIGHT = 20;         // FreeMono12pt character height
    static const int LINE_SPACING = 4;         // Pixels between lines
    
    // Screen dimensions
    static const int TERMINAL_WIDTH = 800;     // Full display width
    static const int TERMINAL_HEIGHT = 24;    // Single line height (CHAR_HEIGHT + LINE_SPACING)
    
    // Colors (RGB565 format)
    static const uint16_t BG_COLOR = 0x0000;     // Black background
    static const uint16_t TEXT_COLOR = 0x07FF;   // Cyan text (default)
    static const uint16_t GREEN_COLOR = 0x07E0;  // Green for success/system
    static const uint16_t YELLOW_COLOR = 0xFFE0; // Yellow for warnings/commands
    static const uint16_t RED_COLOR = 0xF800;    // Red for errors/alerts
    static const uint16_t BLUE_COLOR = 0x001F;   // Blue for info/status
    
    TerminalDisplay();
    ~TerminalDisplay();
    
    void initialize();
    void clear();
    void println(const char* text, uint16_t color = TEXT_COLOR);
    void print(const char* text, uint16_t color = TEXT_COLOR);
    void refresh();  // Redraw all visible content
    
private:
    // Display hardware
    GigaDisplay_GFX tft;
    
    // Text content buffers
    char lines[MAX_LINES][MAX_LINE_LENGTH + 1];  // +1 for null terminator
    uint16_t lineColors[MAX_LINES];              // Color for each line
    int currentLine;
    int currentColumn;
    bool screenFull;  // True when all lines are filled
    
    // Double-buffering for flicker-free rendering
    uint16_t* lineBuffer;                        // Single line pixel buffer
    GFXcanvas16* canvas;                         // Reusable text rendering canvas
    static const int LINE_BUFFER_SIZE = TERMINAL_WIDTH * TERMINAL_HEIGHT;
    
    // Internal rendering methods
    void drawLine(int lineIndex);
    void moveCursorToNextLine();
    int getDisplayY(int lineIndex);
    void renderLineToBuffer(const char* text, uint16_t color);
    void blitLineToScreen(int lineIndex);
};