SECTION "Bootcode", ROM0[$0]

start:
	ld sp, $FFFE

	xor a
	ld hl, $8000

clear_vram_loop:
	ldi [hl], a
	bit 5, h
	jr z, clear_vram_loop

	ld a, $80
	ldh [$26], a
	ldh [$11], a
	ld a, $F3
	ldh [$12], a
	ldh [$25], a
	ld a, $77
	ldh [$24], a

	ld a, $54
	ldh [$47], a

	ld de, $104
	ld hl, $8010

load_logo:
	ld a, [de]
	ld b, a
	call double_bits_and_write_row
	call double_bits_and_write_row
	inc de
	ld a, e
	xor $34
	jr nz, load_logo

	ld de, trademark_symbol
	ld c, $08

load_trademark_symbol:
	ld a, [de]
	inc de
	ldi [hl], a
	inc hl
	dec c
	jr nz, load_trademark_symbol

	ld a, $19
	ld [$9910], a
	ld hl, $992F
	ld c, $c

tilemap_loop:
	dec a
	jr z, tilemap_done
	ldd [hl], a
	dec c
	jr nz, tilemap_loop
	ld l, $0F
	jr tilemap_loop

tilemap_done:
	ld a, 30
	ldh [$42], a

	ld a, $91
	ldh [$40], a
	ld d, $89
	ld c, 15

animate:
	call wait_frame
	ld a, d
	sra a
	sra a
	ldh [$42], a
	ld a, d
	add c
	ld d, a
	ld a, c
	cp 8
	jr nz, no_palette_change
	ld a, $A8
	ldh [$47], a

no_palette_change:
	dec c
	jr nz, animate
	ld a, $FC
	ldh [$47], a

	ld a, $83
	call play_sound
	ld b, 5
	call wait_b_frames
	ld a, $C1
	call play_sound

	ld b, 60
	call wait_b_frames

	ld hl, $01B0
	push hl
	pop af
	ld hl, $014D
	ld bc, $0013
	ld de, $00D8

	jp boot_game

double_bits_and_write_row:
	ld a, 4
	ld c, 0

double_current_bit:
	sla b
	push af
	rl c
	pop af
	rl c
	dec a
	jr nz, double_current_bit
	ld a, c
	ldi [hl], a
	inc hl
	ldi [hl], a
	inc hl
	ret

wait_frame:
	push hl
	ld hl, $FF0F
	res 0, [hl]
wait:
	bit 0, [hl]
	jr z, wait
	pop hl
	ret

wait_b_frames:
	call wait_frame
	dec b
	jr nz, wait_b_frames
	ret

play_sound:
	ldh [$13], a
	ld a, $87
	ldh [$14], a
	ret

trademark_symbol:
db $3C,$42,$B9,$A5,$B9,$A5,$42,$3C

SECTION "Boot Game", ROM0[$FE]

boot_game:
	ldh [$50], a
	
	