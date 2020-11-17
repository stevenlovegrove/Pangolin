if(EMSCRIPTEN)
    macro( create_host_index_html filename prog_name)
        file( WRITE ${filename}
"<!doctype html>
<head>
    <meta charset=\"utf-8\">
    <title>${prog_name}</title>
</head>
<body oncontextmenu=\"return false;\">
    <canvas id=\"canvas\" tabindex=-1></canvas>
    <script type=\"text/javascript\">
        var canvas = document.getElementById(\"canvas\");
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
        var Module = {};
        Module.canvas = canvas;
        Module.arguments = ['test://'];
        window.onerror = function(message, source, lineno, colno, error) {
          console.log(Module.pango_get_exception_message(error));
          return false;
        };
        window.addEventListener(\"unhandledrejection\", function(promiseRejectionEvent) {
            console.log(Module.pango_get_exception_message(promiseRejectionEvent.reason));
        });
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
