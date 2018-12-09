if(EMSCRIPTEN)
    macro( create_host_index_html filename prog_name)
        file( WRITE ${filename}
"<!doctype html>
<head>
    <meta charset=\"utf-8\">
    <title>${prog_name}</title>
</head>
<body oncontextmenu=\"return false;\">
    <canvas id=\"canvas\"></canvas>
    <script type=\"text/javascript\">
        var canvas = document.getElementById(\"canvas\");
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
        var Module = {};
        Module.canvas = canvas;
    </script>
    <script type=\"text/javascript\" src=\"${prog_name}.js\"></script>
</body>
</html>")
    endmacro()

    # Override add_executable to make webpage instead
    macro( add_executable prog_name)
        # Create manifest required for APK
        create_host_index_html("${CMAKE_CURRENT_BINARY_DIR}/index.html" "${prog_name}")
        _add_executable(${prog_name} ${ARGN})
    endmacro()
endif()
