
CXX =			g++

CXXFLAGS =		-O2 -Wall -Wextra -Werror

VULKAN_PATH =	${subst \,/,${VULKAN_SDK}}

INCLUDES =		-I"${VULKAN_PATH}/Include" -Ilibs/glfw/include

LDFLAGS =		-L"${VULKAN_PATH}/Lib" -Llibs/glfw/lib-mingw-w64

LIBS =			-lvulkan-1 -lglfw3 -lgdi32 -luser32 -lkernel32

SRCS =			srcs/main.cpp

OBJS =			${SRCS:.cpp=.o}

NAME =			vulkan.exe

all: 			${NAME}

${NAME}: 		${OBJS}
				${CXX} ${OBJS} -o $@ ${LDFLAGS} ${LIBS}

%.o:			%.cpp
				${CXX} ${CXXFLAGS} ${INCLUDES} -c $< -o $@

clean:
				rm -f srcs/*.o ${NAME}

re: 			clean all

.PHONY: all clean re
