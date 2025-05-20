#pragma once
#include <memory>
#include <GL/gl.h>
#include <GL/glu.h>
#include "../models/Model3D.h"

class ModelRenderer {
private:
    std::shared_ptr<Model3D> model;
    float rotationX = 0.0f;
    float rotationY = 0.0f;
    float zoom = -5.0f;
    GLenum polygonMode = GL_FILL;

public:
    ModelRenderer() = default;

    void setModel(std::shared_ptr<Model3D> newModel) {
        model = std::move(newModel);
    }

    void setPolygonMode(GLenum mode) {
        polygonMode = mode;
    }

    void initialize() {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        GLfloat lightPosition[] = { 10.0f, 10.0f, 10.0f, 1.0f };
        GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

        GLfloat materialAmbient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
        GLfloat materialDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat materialShininess[] = { 50.0f };

        glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

        glEnable(GL_NORMALIZE);
    }

    void resize(int width, int height) {
        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);
        gluPerspective(45.0f, aspectRatio, 0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void updateRotation(float deltaX, float deltaY) {
        rotationY += deltaX * 0.5f;
        rotationX += deltaY * 0.5f;
    }

    void updateZoom(float deltaZ) {
        zoom += deltaZ * 0.1f;
    }

    void render() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glTranslatef(0.0f, 0.0f, zoom);
        glRotatef(rotationX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotationY, 0.0f, 1.0f, 0.0f);

        glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

        if (model) {
            for (const auto& mesh : model->getMeshes()) {
                renderMesh(mesh);
            }
        } else {
            renderCube();
        }
    }

private:
    void renderMesh(const std::shared_ptr<Mesh>& mesh) {
        const auto& vertices = mesh->getVertices();
        const auto& faces = mesh->getFaces();

        glDisable(GL_COLOR_MATERIAL);

        glBegin(GL_TRIANGLES);

        for (size_t i = 0; i < vertices.size(); i += 3) {
            for (size_t j = 0; j < 3 && i + j < vertices.size(); ++j) {
                const auto& vertex = vertices[i + j];

                if (vertex.nx != 0.0f || vertex.ny != 0.0f || vertex.nz != 0.0f) {
                    glNormal3f(vertex.nx, vertex.ny, vertex.nz);
                }

                if (vertex.u != 0.0f || vertex.v != 0.0f) {
                    glTexCoord2f(vertex.u, vertex.v);
                }

                glVertex3f(vertex.x, vertex.y, vertex.z);
            }
        }

        glEnd();
    }

    void renderCube() {
        GLfloat materialDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);

        glBegin(GL_QUADS);

        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);

        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);

        glEnd();
    }
};