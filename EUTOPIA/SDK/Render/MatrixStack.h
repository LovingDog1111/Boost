#pragma once
#include <stack>
#include "../../Utils/Maths.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MatrixStack {
public:
    std::stack<glm::mat4x4> stack;
    bool isDirty;

public:
    MatrixStack() : isDirty(false) {
 
        stack.push(glm::mat4(1.0f));
    }


    void push() {
        try {
            if (!stack.empty()) {
                stack.push(stack.top());
            } else {
                stack.push(glm::mat4(1.0f));
            }
        } catch (...) {
            // 如果出现异常，推入单位矩阵作为备选
            stack.push(glm::mat4(1.0f));
        }
        isDirty = true;
    }

    void pop() {
        if (stack.size() > 1) { 
            stack.pop();
            isDirty = true;
        }
    }


    glm::mat4& top() {
        if (stack.empty()) {
            stack.push(glm::mat4(1.0f));
        }
        return stack.top();
    }

    const glm::mat4& top() const {
        static glm::mat4 identity(1.0f);
        if (stack.empty()) {
            return identity;
        }
        return stack.top();
    }


    void translate(const glm::vec3& translation) {
        if (!stack.empty()) {
            stack.top() = glm::translate(stack.top(), translation);
            isDirty = true;
        }
    }

    void rotate(float angle, const glm::vec3& axis) {
        if (!stack.empty()) {
            stack.top() = glm::rotate(stack.top(), angle, axis);
            isDirty = true;
        }
    }

    void scale(const glm::vec3& scaling) {
        if (!stack.empty()) {
            stack.top() = glm::scale(stack.top(), scaling);
            isDirty = true;
        }
    }

    void multiply(const glm::mat4& matrix) {
        if (!stack.empty()) {
            stack.top() = stack.top() * matrix;
            isDirty = true;
        }
    }


    void loadIdentity() {
        if (!stack.empty()) {
            stack.top() = glm::mat4(1.0f);
            isDirty = true;
        }
    }


    void loadMatrix(const glm::mat4& matrix) {
        if (!stack.empty()) {
            stack.top() = matrix;
            isDirty = true;
        }
    }


    size_t size() const {
        return stack.size();
    }


    bool empty() const {
        return stack.empty();
    }


    void clearDirty() {
        isDirty = false;
    }


    void reset() {
        while (!stack.empty()) {
            stack.pop();
        }
        stack.push(glm::mat4(1.0f));
        isDirty = true;
    }
};