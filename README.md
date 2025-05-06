#La configuración de las librerías es la siguiente:
c/c++ -> General -> “Directorios de Inclusión Adicionales":
$(SolutionDir)/External Libraries/GLEW/include
$(SolutionDir)/External Libraries/GLFW/include
$(SolutionDir)/External Libraries/glm
$(SolutionDir)/External Libraries/assimp/include

Vinculador -> General -> “Directorios de Inclusión Adicionales":
$(SolutionDir)/External Libraries/GLEW/lib/Release/Win32
$(SolutionDir)/External Libraries/GLFW/lib-vc2015
$(SolutionDir)/External Libraries/SOIL2/lib
$(SolutionDir)/External Libraries/assimp/lib

Vinculador -> Entrada -> “Dependencias Adicionales":
opengl32.lib
glew32.lib
glfw3.lib
assimp-vc140-mt.lib
soil2-debug.lib
