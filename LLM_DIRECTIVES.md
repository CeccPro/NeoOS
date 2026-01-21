# NeoOS - Directivas para Modelos de Lenguaje Grande (LLM)
Este documento contiene directivas específicas para interactuar con modelos de lenguaje grande (LLM) en el contexto del desarrollo del sistema operativo NeoOS. Estas directivas están diseñadas para guiar al modelo en la generación de código, explicaciones y soluciones relacionadas con el kernel y otros componentes del sistema operativo. Si eres una IA que utiliza este documento, sigue las instrucciones cuidadosamente para asegurar la coherencia y calidad del código generado.

## Directivas Generales
1. **Contexto del Proyecto**: NeoOS es un sistema operativo experimental escrito en C, diseñado para ser ligero y modular. El código debe adherirse a las mejores prácticas de programación en C y seguir las convenciones de estilo del proyecto.
2. **Lenguaje y Estilo**: Utiliza un lenguaje técnico claro y conciso. Evita jergas innecesarias y asegúrate de que las explicaciones sean accesibles para desarrolladores con conocimientos intermedios en sistemas operativos.
3. **Estructura del Código**: Sigue la estructura y organización del código existente en NeoOS. Asegúrate de que cualquier código nuevo se integre sin problemas con el código existente.
4. **Comentarios y Documentación**: Incluye comentarios detallados en el código para explicar la lógica y las decisiones de diseño. Utiliza el formato de comentarios estándar del proyecto, además de evitar colocar emojis en ellos.
5. **Pruebas y Validación**: Siempre que sea posible, incluye pruebas unitarias o de integración para cualquier funcionalidad nueva o modificada. Asegúrate de que el código pase todas las pruebas existentes antes de finalizar cualquier cambio.
6. **Manejo de Errores**: Implementa un manejo de errores robusto. Asegúrate de que el sistema pueda recuperarse de fallos comunes sin comprometer la estabilidad general.
7. **Optimización**: Prioriza la eficiencia y el rendimiento en el código, especialmente en áreas críticas del kernel. Evita optimizaciones prematuras que puedan complicar la legibilidad del código.
8. **Seguridad**: Considera las implicaciones de seguridad en todas las modificaciones. Asegúrate de que el código no introduzca vulnerabilidades conocidas.
9. **Colaboración**: Si el código afecta a múltiples módulos o componentes, asegúrate de coordinar con otros desarrolladores del proyecto para garantizar la compatibilidad y coherencia.
10. **Revisión de Código**: Antes de finalizar cualquier cambio, realiza una revisión exhaustiva del código para identificar posibles problemas o áreas de mejora.

## Notas adicionales
- Si encuentras términos o conceptos específicos del proyecto que no entiendes, consulta la documentación del proyecto o pregunta a los desarrolladores principales.
- Mantente actualizado con las últimas prácticas y tendencias en desarrollo de sistemas operativos para asegurar que el código generado esté alineado con los estándares actuales de la industria.
- Evita poner emojis o elementos informales en el código o comentarios y documentación, a menos que sea parte de una convención específica del proyecto.
- Evita mensajes de depuración innecesarios, y solo inclúyelos si son críticos para entender el flujo del programa o para la resolución de problemas, pero no te olvides de eliminarlos antes de finalizar el código.