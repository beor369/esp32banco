#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include "saveInjector/saveInjector.h"

class GlobalInjector {
private:
    InjectorData injector;

public:
    // Constructor por defecto
    GlobalInjector() = default;

    // Getter para obtener el objeto InjectorData
    InjectorData getInjector() const {
        Serial.println("obtener un objeto");
        return injector;
    }

    // Setter para modificar el objeto InjectorData
    void setInjector(const InjectorData& newInjector) {
        Serial.println("setter modificar inyetores");
        injector = newInjector;
    }
};
extern GlobalInjector globalInjector;
#endif // GLOBAL_VARIABLES_H
