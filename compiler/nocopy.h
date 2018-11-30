#ifndef NOCOPY_H
#define NOCOPY_H

// Inheriting from this class easily disables instance copying
class NoCopy {
public:
    NoCopy(const NoCopy &rhs) = delete;
    NoCopy &operator=(const NoCopy &rhs) = delete;

    NoCopy(NoCopy &&rhs) {
    }

    NoCopy &operator=(NoCopy &&rhs) {
        return *this;
    }

    NoCopy() = default;
    ~NoCopy() = default;
};

#endif // NOCOPY_H
