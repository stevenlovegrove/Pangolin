macro(PangolinCreateFactoryRegisterFile directory namespace)
    foreach(interface_name ${PANGO_FACTORY_INTERFACES})
        set(filename "${directory}/RegisterFactories${interface_name}.h")

        file(WRITE ${filename} "// CMake generated file. Do Not Edit.\n\n#pragma once\n\nnamespace ${namespace} {\n\n")

        file(APPEND ${filename} "  // Forward declarations\n")
#        file(APPEND ${filename} "  class ${interface_name};\n")
        foreach(factory_name ${PANGO_FACTORY_${interface_name}})
            file(APPEND ${filename} "  bool Register${factory_name}Factory();\n")
        endforeach()

        file(APPEND ${filename} "\n\n")

        file(APPEND ${filename} "  void RegisterFactories${interface_name}() {\n")
        file(APPEND ${filename} "    static bool inited = false;\n")
        file(APPEND ${filename} "    if(inited) return;\n")
        file(APPEND ${filename} "    inited = true;\n")

        foreach(factory_name ${PANGO_FACTORY_${interface_name}})
            file(APPEND ${filename} "    Register${factory_name}Factory();\n")
        endforeach()

        file(APPEND ${filename} "  }\n")

        file(APPEND ${filename} "\n\n} // ${namespace}\n")
    endforeach()
endmacro()

# Provide factory names as list of arguments
macro( PangolinRegisterFactory interface_name)
    list(APPEND PANGO_FACTORY_INTERFACES "${interface_name}")
    foreach(arg ${ARGN})
        list(REMOVE_DUPLICATES PANGO_FACTORY_INTERFACES)
        list(APPEND PANGO_FACTORY_${interface_name} "${arg}")
    endforeach()

endmacro()

