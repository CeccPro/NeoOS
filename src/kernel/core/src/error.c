/*
 * NeoOS - Error Codes
 * Definiciones de códigos de error para el kernel
 */

#include "../include/error.h"

/**
 * Convierte un código de error a su nombre como string
 * @param error Código de error
 * @return String con el nombre del error
 */
 const char* error_to_string(int error) {
    switch (error) {
        case E_OK:            return "E_OK";
        case E_UNKNOWN:       return "E_UNKNOWN";
        case E_NOMEM:         return "E_NOMEM";
        case E_INVAL:         return "E_INVAL";
        case E_NOENT:         return "E_NOENT";
        case E_EXISTS:        return "E_EXISTS";
        case E_BUSY:          return "E_BUSY";
        case E_IO:            return "E_IO";
        case E_PERM:          return "E_PERM";
        case E_TIMEOUT:       return "E_TIMEOUT";
        case E_MODULE_ERR:    return "E_MODULE_ERR";
        case E_NOT_IMPL:      return "E_NOT_IMPL";
        case E_NOT_SUPPORTED: return "E_NOT_SUPPORTED";
        default:              return "E_UNKNOWN";
    }
}