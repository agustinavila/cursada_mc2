################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cr_startup_lpc43xx.c \
../src/crp.c \
../src/led_Driver.c \
../src/main.c \
../src/sysinit.c \
../src/teclado.c \
../src/teclasPinInt.c \
../src/teclas_driver.c 

C_DEPS += \
./src/cr_startup_lpc43xx.d \
./src/crp.d \
./src/led_Driver.d \
./src/main.d \
./src/sysinit.d \
./src/teclado.d \
./src/teclasPinInt.d \
./src/teclas_driver.d 

OBJS += \
./src/cr_startup_lpc43xx.o \
./src/crp.o \
./src/led_Driver.o \
./src/main.o \
./src/sysinit.o \
./src/teclado.o \
./src/teclasPinInt.o \
./src/teclas_driver.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DNDEBUG -D__CODE_RED -DCORE_M4 -D__USE_LPCOPEN -DNO_BOARD_LIB -D__LPC43XX__ -D__REDLIB__ -I"C:\Users\agust\Documents\cursada_mc2\inc" -I"C:\Users\agust\Documents\MCUXpressoIDE_11.7.0_9198\workspace\lpc_chip_43xx\inc" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -fsingle-precision-constant -fmacro-prefix-map="$(<D)/"= -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=softfp -mthumb -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/cr_startup_lpc43xx.d ./src/cr_startup_lpc43xx.o ./src/crp.d ./src/crp.o ./src/led_Driver.d ./src/led_Driver.o ./src/main.d ./src/main.o ./src/sysinit.d ./src/sysinit.o ./src/teclado.d ./src/teclado.o ./src/teclasPinInt.d ./src/teclasPinInt.o ./src/teclas_driver.d ./src/teclas_driver.o

.PHONY: clean-src

