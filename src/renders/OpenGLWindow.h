#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Text_Display.H>
#include <memory>
#include <string>
#include "ModelRenderer.h"
#include "../models/ModelManager.h"
#include "../analyzer/MeshQualityAnalyzer.h"

class GLWindow : public Fl_Gl_Window {
private:
    bool mouseDown = false;
    int lastMouseX = 0;
    int lastMouseY = 0;

public:
    GLWindow(int x, int y, int w, int h) : Fl_Gl_Window(x, y, w, h, "3D Viewer") {
        renderer = std::make_unique<ModelRenderer>();
        mode(FL_RGB | FL_DOUBLE | FL_DEPTH);
    }

    void setModel(std::shared_ptr<Model3D> model) {
        renderer->setModel(std::move(model));
        redraw();
    }

    std::unique_ptr<ModelRenderer> renderer;
protected:
    void draw() override {
        if (!valid()) {
            renderer->initialize();
            renderer->resize(w(), h());
            valid(1);
        }
        renderer->render();
    }

    int handle(int event) override {
        switch (event) {
        case FL_PUSH:
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                mouseDown = true;
                lastMouseX = Fl::event_x();
                lastMouseY = Fl::event_y();
                return 1;
            }
            break;

        case FL_RELEASE:
            if (Fl::event_button() == FL_LEFT_MOUSE) {
                mouseDown = false;
                return 1;
            }
            break;

        case FL_DRAG:
            if (mouseDown) {
                int x = Fl::event_x();
                int y = Fl::event_y();
                renderer->updateRotation(static_cast<float>(x - lastMouseX), static_cast<float>(y - lastMouseY));
                lastMouseX = x;
                lastMouseY = y;
                redraw();
                return 1;
            }
            break;

        case FL_MOUSEWHEEL:
            renderer->updateZoom(static_cast<float>(Fl::event_dy()) * -1.0f);
            redraw();
            return 1;
        }
        return Fl_Gl_Window::handle(event);
    }
};

class OpenGLWindow : public Fl_Window {
private:
    Fl_Menu_Bar* menu;
    GLWindow* gl_window;
    ModelManager modelManager;
    std::shared_ptr<Model3D> currentModel;

public:
    OpenGLWindow(int w, int h) : Fl_Window(w, h, "3D Model Viewer") {
        menu = new Fl_Menu_Bar(0, 0, w, 30);
        menu->add("File/Open", 0, open_cb, this);
        menu->add("Display Mode/Solid", 0, solid_cb, this);
        menu->add("Display Mode/Wireframe", 0, wireframe_cb, this);
        //menu->add("Analysis/Analyze", 0, analyze_cb, this);
        gl_window = new GLWindow(0, 30, w, h - 30);
        resizable(gl_window);
        Fl::add_timeout(1.0 / 60.0, timer_cb, this);
    }

    void setModel(std::shared_ptr<Model3D> model) {
        currentModel = model;
        gl_window->setModel(model);
    }

private:
    static void open_cb(Fl_Widget*, void* v) {
        auto* win = (OpenGLWindow*)v;
        Fl_File_Chooser chooser(".", "*.obj", Fl_File_Chooser::SINGLE, "Choose OBJ file");
        chooser.show();
        while (chooser.visible())
            Fl::wait();
        if (chooser.value()) {
            try {
                auto model = win->modelManager.loadModel(chooser.value());
                win->setModel(model);
            } catch (const std::exception& e) {
                Fl::error("Model Loading Error: %s", e.what());
            }
        }
    }

    static void solid_cb(Fl_Widget*, void* v) {
        OpenGLWindow* win = (OpenGLWindow*)v;
        win->gl_window->renderer->setPolygonMode(GL_FILL);
        win->gl_window->redraw();
    }

    static void wireframe_cb(Fl_Widget*, void* v) {
        OpenGLWindow* win = (OpenGLWindow*)v;
        win->gl_window->renderer->setPolygonMode(GL_LINE);
        win->gl_window->redraw();
    }

    static void analyze_cb(Fl_Widget*, void* v) {
        OpenGLWindow* win = (OpenGLWindow*)v;
        if (!win->currentModel) {
            Fl::error("No model loaded for analysis.");
            return;
        }

        std::string summary;
        for (const auto& mesh : win->currentModel->getMeshes()) {
            MeshQualityAnalyzer analyzer(*mesh);
            analyzer.analyzeQuality();
            summary += analyzer.getQualitySummary() + "\n\n";
        }

        Fl_Window* analysisWindow = new Fl_Window(600, 400, "Mesh Quality Analysis");
        Fl_Text_Buffer* buffer = new Fl_Text_Buffer();
        Fl_Text_Display* display = new Fl_Text_Display(10, 10, 580, 380);
        display->buffer(buffer);
        buffer->text(summary.c_str());
        analysisWindow->resizable(display);
        analysisWindow->end();
        analysisWindow->show();
    }

    static void timer_cb(void* v) {
        OpenGLWindow* win = (OpenGLWindow*)v;
        win->gl_window->redraw();
        Fl::repeat_timeout(1.0 / 60.0, timer_cb, v);
    }
};