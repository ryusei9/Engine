#pragma once

struct Matrix4x4 {
	float m[4][4];

    // 行列の加算
    Matrix4x4 operator+(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = this->m[i][j] + other.m[i][j];
            }
        }
        return result;
    }

    // 行列の減算
    Matrix4x4 operator-(const Matrix4x4& other) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = this->m[i][j] - other.m[i][j];
            }
        }
        return result;
    }

    // 行列の乗算
    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 result = {};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += this->m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }

    // 行列のスカラー乗算
    Matrix4x4 operator*(float scalar) const {
        Matrix4x4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = this->m[i][j] * scalar;
            }
        }
        return result;
    }

    static Matrix4x4 Transpose(const Matrix4x4& m)
    {
        Matrix4x4 result{};
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result.m[i][j] = m.m[j][i];
            }
        }
        return result;
    }
};