# --- BUILD ---

.PHONY : build
build :
	meson compile -C build

vendor/limine/limine :
	$(MAKE) -C vendor/limine limine

.PHONY : iso
iso : build vendor/limine/limine
	./iso.sh

# --- RUN ---

QEMU_ARGS = -drive id=disk,file=cosmos-os.iso,format=raw,if=none -device ide-hd,drive=disk -accel kvm

.PHONY : run
run : iso
	qemu-system-x86_64 $(QEMU_ARGS)

.PHONY : debug
debug : iso
	qemu-system-x86_64 $(QEMU_ARGS) -s -S
