#include "TerminalDisplay.h"
#include <string.h>

TerminalDisplay::TerminalDisplay() 
    : currentLine(0), currentColumn(0), lineBuffer(nullptr), canvas(nullptr), screenFull(false) {
    for (int i = 0; i < MAX_LINES; i++) {
        lines[i][0] = '\0';
        lineColors[i] = TEXT_COLOR;
    }
}

TerminalDisplay::~TerminalDisplay() {
    if (lineBuffer) {
        delete[] lineBuffer;
    }
    if (canvas) {
        delete canvas;
    }
}

void TerminalDisplay::initialize() {
    // Initialize display hardware
    tft.begin();
    tft.setRotation(1);  // Landscape orientation
    tft.setFont(&FreeMono12pt7b);
    tft.setTextColor(TEXT_COLOR);
    
    // Allocate double-buffering memory
    lineBuffer = new uint16_t[LINE_BUFFER_SIZE];
    if (!lineBuffer) {
        Serial.println("ERROR: Failed to allocate line buffer!");
        return;
    }
    
    canvas = new GFXcanvas16(TERMINAL_WIDTH, TERMINAL_HEIGHT);
    if (!canvas) {
        Serial.println("ERROR: Failed to allocate canvas!");
        return;
    }
    canvas->setFont(&FreeMono12pt7b);
    canvas->setTextColor(TEXT_COLOR);
    
    clear();
    Serial.println("TerminalDisplay: Initialized with double buffering");
}

void TerminalDisplay::clear() {
    for (int i = 0; i < MAX_LINES; i++) {
        lines[i][0] = '\0';
        lineColors[i] = TEXT_COLOR;
    }
    
    currentLine = 0;
    currentColumn = 0;
    screenFull = false;
    
    tft.fillScreen(BG_COLOR);
}

void TerminalDisplay::println(const char* text, uint16_t color) {
    // When screen is full, clear and restart on next event
    if (screenFull) {
        tft.fillScreen(BG_COLOR);
        
        for (int i = 0; i < MAX_LINES; i++) {
            lines[i][0] = '\0';
            lineColors[i] = TEXT_COLOR;
        }
        
        currentLine = 0;
        currentColumn = 0;
        screenFull = false;
    }
    
    print(text, color);
    moveCursorToNextLine();
}

void TerminalDisplay::print(const char* text, uint16_t color) {
    if (!text) return;
    
    // Store color for current line
    lineColors[currentLine] = color;
    
    int textLen = strlen(text);
    int pos = 0;
    
    while (pos < textLen) {
        int remainingSpace = MAX_LINE_LENGTH - currentColumn;
        int charsToWrite = min(remainingSpace, textLen - pos);
        
        // Add characters to current line buffer
        strncat(lines[currentLine] + currentColumn, text + pos, charsToWrite);
        currentColumn += charsToWrite;
        pos += charsToWrite;
        
        // Update display immediately
        drawLine(currentLine);
        
        // Move to next line if needed
        if (currentColumn >= MAX_LINE_LENGTH || pos < textLen) {
            moveCursorToNextLine();
            // Set color for new line too if we're continuing
            if (pos < textLen) {
                lineColors[currentLine] = color;
            }
        }
    }
}

void TerminalDisplay::refresh() {
    tft.fillScreen(BG_COLOR);
    
    for (int i = 0; i < MAX_LINES; i++) {
        if (lines[i][0] != '\0') {
            drawLine(i);
        }
    }
}

// =============================================================================
// Internal rendering methods
// =============================================================================

void TerminalDisplay::drawLine(int lineIndex) {
    if (lineIndex < 0 || lineIndex >= MAX_LINES) return;
    
    renderLineToBuffer(lines[lineIndex], lineColors[lineIndex]);
    blitLineToScreen(lineIndex);
}

void TerminalDisplay::moveCursorToNextLine() {
    currentLine++;
    currentColumn = 0;
    
    // When reaching bottom, mark screen as full and stay at last line
    if (currentLine >= MAX_LINES) {
        currentLine = MAX_LINES - 1;
        screenFull = true;
    }
    
    lines[currentLine][0] = '\0';
    lineColors[currentLine] = TEXT_COLOR;  // Default color for new line
}

int TerminalDisplay::getDisplayY(int lineIndex) {
    return lineIndex * (CHAR_HEIGHT + LINE_SPACING) + CHAR_HEIGHT;
}

void TerminalDisplay::renderLineToBuffer(const char* text, uint16_t color) {
    if (!lineBuffer || !canvas) return;
    
    canvas->fillScreen(BG_COLOR);
    
    if (text && text[0] != '\0') {
        canvas->setTextColor(color);
        canvas->setCursor(0, CHAR_HEIGHT);
        canvas->print(text);
    }
    
    // Copy rendered content to line buffer
    uint16_t* canvasBuffer = canvas->getBuffer();
    if (canvasBuffer) {
        memcpy(lineBuffer, canvasBuffer, LINE_BUFFER_SIZE * sizeof(uint16_t));
    }
}

void TerminalDisplay::blitLineToScreen(int lineIndex) {
    if (!lineBuffer || lineIndex < 0 || lineIndex >= MAX_LINES) return;
    
    int y = getDisplayY(lineIndex) - CHAR_HEIGHT;
    tft.drawRGBBitmap(0, y, lineBuffer, TERMINAL_WIDTH, TERMINAL_HEIGHT);
}


