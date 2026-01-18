@echo off
echo Starting CawOS
qemu-system-i386 -drive format=raw,file=os-image.bin -audiodev driver=dsound,id=spk -machine pcspk-audiodev=spk