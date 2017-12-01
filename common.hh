/**
 * common.hh - common definitions
 *
 * Author: Bao Hexing <HexingB@qq.com>
 * Created: 25 November 2017
 *
 * Copyright © 2017, Bao Hexing. All Rights Reserved.
 */

#ifndef TTL_COMMON_H
#define TTL_COMMON_H

#include <string>

namespace ttl {
    bool FileExists(const std::string& filename);
    const char * ReadFile(const std::string& filename);

    class Constants {
    private:
        Constants();
    public:
        const static long DEFAULT_RETURN_VALUE = 0;
    };

    static inline double assign(double lhs, double rhs) {
        return rhs;
    }

    static inline double add(double lhs, double rhs) {
        return lhs + rhs;
    }

    static inline double sub(double lhs, double rhs) {
        return lhs - rhs;
    }

    static inline double mul(double lhs, double rhs) {
        return lhs * rhs;
    }

    static inline double div(double lhs, double rhs) {
        return lhs / rhs;
    }

    static inline double mod(double lhs, double rhs) {
        return (long long) lhs % (long long) rhs;
    }
}

#endif
