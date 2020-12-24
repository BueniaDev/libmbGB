; libmbGB CGB bootstrap ROM (ported from SameBoy, (C) 2020 Lior Halphon)

SECTION "Bootcode", ROM0[$0]

start:
	ld sp, $FFFE

	ld hl, $8000
	call clear_memory_page
	ld a, 2
	ld c, $70
	ld [c], a
	ld h, $D0
	xor a
	call clear_memory_page
	ld [c], a

	ldh [input_palette], a
	ldh [title_checksum], a

	ld h, $FE
	ld c, $A0

clear_oam:
	ldi [hl], a
	dec c
	jr nz, clear_oam

	ld a, $80
	ldh [$26], a
	ldh [$11], a
	ld a, $F3
	ldh [$12], a
	ldh [$25], a
	ld a, $77
	ldh [$24], a

	call init_waveform

	ld a, [$FC]
	ldh [$47], a

	ld de, $104
	ld hl, $8010

load_logo:
	ld a, [de]
	ld b, a
	call double_bits_and_write_row_twice
	inc de
	ld a, e
	cp $34
	jr nz, load_logo
	call read_trademark

	ld a, 1
	ldh [$4F], a
	xor a
	ld hl, $8000
	call clear_memory_page
	call load_tileset

	ld b, 3
	ld hl, $98C2
	ld d, 3
	ld a, 8

tilemap_loop:
	ld c, $10

tilemap_row_loop:
	push af
	ld a, 1
	ldh [$4F], a
	ld [hl], 8

	xor a
	ldh [$4F], a
	pop af

	ldi [hl], a
	add d
	dec c
	jr nz, tilemap_row_loop
	sub 47
	push de
	ld de, $10
	add hl, de
	pop de
	dec b
	jr nz, tilemap_loop

	dec d
	jr z, .end_tilemap
	dec d
	ld a, $38
	ld l, $A7
	ld bc, $0107
	jr tilemap_row_loop
.end_tilemap

	ld de, animation_colors
	ld c, 8
	ld hl, bg_palettes
	xor a
expand_palettes:
	cpl
	ldi [hl], a
	ldi [hl], a

	ld a, [de]
	inc de
	ldi [hl], a
	ld a, [de]
	inc de
	ldi [hl], a

	xor a
	
	ldi [hl], a
	ldi [hl], a
	ldi [hl], a
	ldi [hl], a

	dec c
	jr nz, expand_palettes

	ld hl, bg_palettes
	call load_bg_palettes_64

	ld a, $91
	ldh [$40], a

	call do_intro_animation

	ld b, 45
	call wait_b_frames

	ld a, $83
	call play_sound
	ld b, 5
	call wait_b_frames

	ld a, $C1
	call play_sound

	ld a, 30
	ldh [wait_loop_counter], a

.wait_loop
	call get_input_palette_index
	call wait_frame
	ld hl, wait_loop_counter
	dec [hl]
	jr nz, .wait_loop

	call pre_boot

SECTION "Boot game", ROM0[$FE]

boot_game:
	ldh [$50], a

SECTION "More stuff", ROM0[$200]

title_checksums:
    db $00 ; Default
    db $88 ; ALLEY WAY
    db $16 ; YAKUMAN
    db $36 ; BASEBALL, (Game and Watch 2)
    db $D1 ; TENNIS
    db $DB ; TETRIS
    db $F2 ; QIX
    db $3C ; DR.MARIO
    db $8C ; RADARMISSION
    db $92 ; F1RACE
    db $3D ; YOSSY NO TAMAGO
    db $5C ;
    db $58 ; X
    db $C9 ; MARIOLAND2
    db $3E ; YOSSY NO COOKIE
    db $70 ; ZELDA
    db $1D ;
    db $59 ;
    db $69 ; TETRIS FLASH
    db $19 ; DONKEY KONG
    db $35 ; MARIO'S PICROSS
    db $A8 ;
    db $14 ; POKEMON RED, (GAMEBOYCAMERA G)
    db $AA ; POKEMON GREEN
    db $75 ; PICROSS 2
    db $95 ; YOSSY NO PANEPON
    db $99 ; KIRAKIRA KIDS
    db $34 ; GAMEBOY GALLERY
    db $6F ; POCKETCAMERA
    db $15 ;
    db $FF ; BALLOON KID
    db $97 ; KINGOFTHEZOO
    db $4B ; DMG FOOTBALL
    db $90 ; WORLD CUP
    db $17 ; OTHELLO
    db $10 ; SUPER RC PRO-AM
    db $39 ; DYNABLASTER
    db $F7 ; BOY AND BLOB GB2
    db $F6 ; MEGAMAN
    db $A2 ; STAR WARS-NOA
    db $49 ;
    db $4E ; WAVERACE
    db $43 | $80 ;
    db $68 ; LOLO2
    db $E0 ; YOSHI'S COOKIE
    db $8B ; MYSTIC QUEST
    db $F0 ;
    db $CE ; TOPRANKINGTENNIS
    db $0C ; MANSELL
    db $29 ; MEGAMAN3
    db $E8 ; SPACE INVADERS
    db $B7 ; GAME&WATCH
    db $86 ; DONKEYKONGLAND95
    db $9A ; ASTEROIDS/MISCMD
    db $52 ; STREET FIGHTER 2
    db $01 ; DEFENDER/JOUST
    db $9D ; KILLERINSTINCT95
    db $71 ; TETRIS BLAST
    db $9C ; PINOCCHIO
    db $BD ;
    db $5D ; BA.TOSHINDEN
    db $6D ; NETTOU KOF 95
    db $67 ;
    db $3F ; TETRIS PLUS
    db $6B ; DONKEYKONGLAND 3

first_checksum_with_duplicate:
    ; Let's play hangman!
    db $B3 ; ???[B]????????
    db $46 ; SUP[E]R MARIOLAND
    db $28 ; GOL[F]
    db $A5 ; SOL[A]RSTRIKER
    db $C6 ; GBW[A]RS
    db $D3 ; KAE[R]UNOTAMENI
    db $27 ; ???[B]????????
    db $61 ; POK[E]MON BLUE
    db $18 ; DON[K]EYKONGLAND
    db $66 ; GAM[E]BOY GALLERY2
    db $6A ; DON[K]EYKONGLAND 2
    db $BF ; KID[ ]ICARUS
    db $0D ; TET[R]IS2
    db $F4 ; ???[-]????????
    db $B3 ; MOG[U]RANYA
    db $46 ; ???[R]????????
    db $28 ; GAL[A]GA&GALAXIAN
    db $A5 ; BT2[R]AGNAROKWORLD
    db $C6 ; KEN[ ]GRIFFEY JR
    db $D3 ; ???[I]????????
    db $27 ; MAG[N]ETIC SOCCER
    db $61 ; VEG[A]S STAKES
    db $18 ; ???[I]????????
    db $66 ; MIL[L]I/CENTI/PEDE
    db $6A ; MAR[I]O & YOSHI
    db $BF ; SOC[C]ER
    db $0D ; POK[E]BOM
    db $F4 ; G&W[ ]GALLERY
    db $B3 ; TET[R]IS ATTACK
checksums_end:

palette_per_checksum:
    db 0 	; Default Palette
    db 4 	; ALLEY WAY
    db 5 	; YAKUMAN
    db 35 	; BASEBALL, (Game and Watch 2)
    db 34 	; TENNIS
    db 3 	; TETRIS
    db 31 	; QIX
    db 15 	; DR.MARIO
    db 10 	; RADARMISSION
    db 5 	; F1RACE
    db 19 	; YOSSY NO TAMAGO
    db 36 	;
    db 7 | $80 ; X
    db 37 	; MARIOLAND2
    db 30 	; YOSSY NO COOKIE
    db 44 	; ZELDA
    db 21 	;
    db 32 	;
    db 31 	; TETRIS FLASH
    db 20 	; DONKEY KONG
    db 5 	; MARIO'S PICROSS
    db 33 	;
    db 13 	; POKEMON RED, (GAMEBOYCAMERA G)
    db 14 	; POKEMON GREEN
    db 5 	; PICROSS 2
    db 29 	; YOSSY NO PANEPON
    db 5 	; KIRAKIRA KIDS
    db 18 	; GAMEBOY GALLERY
    db 9 	; POCKETCAMERA
    db 3 	;
    db 2 	; BALLOON KID
    db 26 	; KINGOFTHEZOO
    db 25 	; DMG FOOTBALL
    db 25 	; WORLD CUP
    db 41 	; OTHELLO
    db 42 	; SUPER RC PRO-AM
    db 26 	; DYNABLASTER
    db 45 	; BOY AND BLOB GB2
    db 42 	; MEGAMAN
    db 45 	; STAR WARS-NOA
    db 36 	;
    db 38 	; WAVERACE
    db 26 	;
    db 42 	; LOLO2
    db 30 	; YOSHI'S COOKIE
    db 41 	; MYSTIC QUEST
    db 34 	;
    db 34 	; TOPRANKINGTENNIS
    db 5 	; MANSELL
    db 42 	; MEGAMAN3
    db 6 	; SPACE INVADERS
    db 5 	; GAME&WATCH
    db 33 	; DONKEYKONGLAND95
    db 25 	; ASTEROIDS/MISCMD
    db 42 	; STREET FIGHTER 2
    db 42 	; DEFENDER/JOUST
    db 40 	; KILLERINSTINCT95
    db 2 	; TETRIS BLAST
    db 16 	; PINOCCHIO
    db 25 	;
    db 42 	; BA.TOSHINDEN
    db 42 	; NETTOU KOF 95
    db 5 	;
    db 0 	; TETRIS PLUS
    db 39 	; DONKEYKONGLAND 3
    db 36 	;
    db 22 	; SUPER MARIOLAND
    db 25 	; GOLF
    db 6 	; SOLARSTRIKER
    db 32 	; GBWARS
    db 12 	; KAERUNOTAMENI
    db 36 	;
    db 11 	; POKEMON BLUE
    db 39 	; DONKEYKONGLAND
    db 18 	; GAMEBOY GALLERY2
    db 39 	; DONKEYKONGLAND 2
    db 24 	; KID ICARUS
    db 31 	; TETRIS2
    db 50 	;
    db 17 	; MOGURANYA
    db 46 	;
    db 6 	; GALAGA&GALAXIAN
    db 27 	; BT2RAGNAROKWORLD
    db 0 	; KEN GRIFFEY JR
    db 47 	;
    db 41 	; MAGNETIC SOCCER
    db 41 	; VEGAS STAKES
    db 0 	;
    db 0 	; MILLI/CENTI/PEDE
    db 19 	; MARIO & YOSHI
    db 34 	; SOCCER
    db 23 	; POKEBOM
    db 18 	; G&W GALLERY
    db 29 	; TETRIS ATTACK

dups_4th_letter_array:
    db "BEFAARBEKEK R-URAR INAILICE R"

palette_combinations:
palette_comb: MACRO ; Obj0, Obj1, Bg
    db (\1) * 8, (\2) * 8, (\3) *8
ENDM
raw_palette_comb: MACRO ; Obj0, Obj1, Bg
    db (\1) * 2, (\2) * 2, (\3) * 2
ENDM
    palette_comb 4, 4, 29
    palette_comb 18, 18, 18
    palette_comb 20, 20, 20
    palette_comb 24, 24, 24
    palette_comb 9, 9, 9
    palette_comb 0, 0, 0
    palette_comb 27, 27, 27
    palette_comb 5, 5, 5
    palette_comb 12, 12, 12
    palette_comb 26, 26, 26
    palette_comb 16, 8, 8
    palette_comb 4, 28, 28
    palette_comb 4, 2, 2
    palette_comb 3, 4, 4
    palette_comb 4, 29, 29
    palette_comb 28, 4, 28
    palette_comb 2, 17, 2
    palette_comb 16, 16, 8
    palette_comb 4, 4, 7
    palette_comb 4, 4, 18
    palette_comb 4, 4, 20
    palette_comb 19, 19, 9
    raw_palette_comb 4 * 4 - 1, 4 * 4 - 1, 11 * 4
    palette_comb 17, 17, 2
    palette_comb 4, 4, 2
    palette_comb 4, 4, 3
    palette_comb 28, 28, 0
    palette_comb 3, 3, 0
    palette_comb 0, 0, 1
    palette_comb 18, 22, 18
    palette_comb 20, 22, 20
    palette_comb 24, 22, 24
    palette_comb 16, 22, 8
    palette_comb 17, 4, 13
    raw_palette_comb 28 * 4 - 1, 0 * 4, 14 * 4
    raw_palette_comb 28 * 4 - 1, 4 * 4, 15 * 4
    palette_comb 19, 22, 9
    palette_comb 16, 28, 10
    palette_comb 4, 23, 28
    palette_comb 17, 22, 2
    palette_comb 4, 0, 2
    palette_comb 4, 28, 3
    palette_comb 28, 3, 0
    palette_comb 3, 28, 4
    palette_comb 21, 28, 4
    palette_comb 3, 28, 0
    palette_comb 25, 3, 28
    palette_comb 0, 28, 8
    palette_comb 4, 3, 28
    palette_comb 28, 3, 6
    palette_comb 4, 28, 29

palettes:
    dw $7FFF, $32BF, $00D0, $0000
    dw $639F, $4279, $15B0, $04CB
    dw $7FFF, $6E31, $454A, $0000
    dw $7FFF, $1BEF, $0200, $0000
    dw $7FFF, $421F, $1CF2, $0000
    dw $7FFF, $5294, $294A, $0000
    dw $7FFF, $03FF, $012F, $0000
    dw $7FFF, $03EF, $01D6, $0000
    dw $7FFF, $42B5, $3DC8, $0000
    dw $7E74, $03FF, $0180, $0000
    dw $67FF, $77AC, $1A13, $2D6B
    dw $7ED6, $4BFF, $2175, $0000
    dw $53FF, $4A5F, $7E52, $0000
    dw $4FFF, $7ED2, $3A4C, $1CE0
    dw $03ED, $7FFF, $255F, $0000
    dw $036A, $021F, $03FF, $7FFF
    dw $7FFF, $01DF, $0112, $0000
    dw $231F, $035F, $00F2, $0009
    dw $7FFF, $03EA, $011F, $0000
    dw $299F, $001A, $000C, $0000
    dw $7FFF, $027F, $001F, $0000
    dw $7FFF, $03E0, $0206, $0120
    dw $7FFF, $7EEB, $001F, $7C00
    dw $7FFF, $3FFF, $7E00, $001F
    dw $7FFF, $03FF, $001F, $0000
    dw $03FF, $001F, $000C, $0000
    dw $7FFF, $033F, $0193, $0000
    dw $0000, $4200, $037F, $7FFF
    dw $7FFF, $7E8C, $7C00, $0000
    dw $7FFF, $1BEF, $6180, $0000

key_combo_palettes:
    db 1 ; Right
    db 48 ; Left
    db 5 ; Up
    db 8 ; Down
    db 0 ; Right + A
    db 40 ; Left + A
    db 43 ; Up + A
    db 3 ; Down + A
    db 6 ; Right + B
    db 7 ; Left + B
    db 28 ; Up + B
    db 49 ; Down + B

trademark_symbol:
    db $3c,$42,$b9,$a5,$b9,$a5,$42,$3c

libmbgb_logo:
	incbin "libmbgb_logo.1bpp"
libmbgb_logo_end:

animation_colors:
    dw $7FFF ; White
    dw $774F ; Cyan
    dw $22C7 ; Green
    dw $039F ; Yellow
    dw $017D ; Orange
    dw $241D ; Red
    dw $6D38 ; Purple
    dw $7102 ; Blue
AnimationColorsEnd:

dmg_palettes:
    dw $7FFF, $32BF, $00D0, $0000

double_bits_and_write_row_twice:
	call .twice

.twice
	ld a, 4
	ld c, 0

.double_current_bit
	sla b
	push af
	rl c
	pop af
	rl c
	dec a
	jr nz, .double_current_bit
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
.wait
	bit 0, [hl]
	jr z, .wait
	pop hl
	ret

wait_b_frames:
	call get_input_palette_index
	call wait_frame
	dec b
	jr nz, wait_b_frames
	ret

play_sound:
	ldh [$13], a
	ld a, $87
	ldh [$14], a
	ret

clear_memory_page:
	ldi [hl], a
	bit 5, h
	jr z, clear_memory_page
	ret

read_two_tile_lines:
	call read_tile_line
read_tile_line:
	ld a, [de]
	and c
	ld b, a
	inc e
	inc e
	ld a, [de]
	dec e
	dec e
	and c
	swap a
	or b
	bit 0, c
	jr z, .dont_swap
	swap a
.dont_swap
	inc hl
	ldi [hl], a
	swap c
	ret

read_cgb_logo_half_tile:
	call .do_twice
.do_twice
	call read_two_tile_lines
	inc e
	ld a, e
	ret

load_tileset:
	ld de, libmbgb_logo
	ld hl, $8080
	ld c, (libmbgb_logo_end - libmbgb_logo) / 2
.libmbgb_logo_loop
	ld a, [de]
	ldi [hl], a
	inc hl
	inc de
	ld a, [de]
	ldi [hl], a
	inc hl
	inc de
	dec c
	jr nz, .libmbgb_logo_loop

	ld de, $104
.cgb_rom_logo_loop
	ld c, $F0
	call read_cgb_logo_half_tile
	add a, 22
	ld e, a
	call read_cgb_logo_half_tile
	sub a, 22
	ld e, a
	cp $1c
	jr nz, .cgb_rom_logo_loop
	inc hl
	
read_trademark:
	ld de, trademark_symbol
	ld c, $08

.load_trademark:
	ld a, [de]
	inc de
	ldi [hl], a
	inc hl
	dec c
	jr nz, .load_trademark
	ret

load_obj_palettes:
	ld c, $6A
	jr load_palettes

load_bg_palettes_64:
	ld d, 64

load_bg_palettes:
	ld e, 0
	ld c, $68

load_palettes:
	ld a, $80
	or e
	ld [c], a
	inc c
.loop
	ld a, [hli]
	ld [c], a
	dec d
	jr nz, .loop
	ret

do_intro_animation:
	ld a, 1
	ldh [$4F], a
	ld d, 26
.animation_loop:
	ld b, 2
	call wait_b_frames
	ld hl, $98C0
	ld c, 3
.loop
	ld a, [hl]
	cp $F
	jr z, .next_tile
	inc [hl]
	and $7
	jr z, .next_line

.next_tile
	inc hl
	jr .loop

.next_line
	ld a, l
	or $1F
	ld l, a
	inc hl
	dec c
	jr nz, .loop
	dec d
	jr nz, .animation_loop
	ret

pre_boot:
	ld b, 32
.fade_loop
	ld c, 32
	ld hl, bg_palettes

.frame_loop
	push bc
	call brighten_color
	pop bc
	dec c
	jr nz, .frame_loop
	call wait_frame
	call wait_frame
	ld hl, bg_palettes
	call load_bg_palettes_64
	dec b
	jr nz, .fade_loop

	call clear_vram_via_hdma
	xor a
	ldh [$4F], a
	cpl
	ldh [$00], a
	call clear_vram_via_hdma

	ld de, $FF56
	ld l, $0D
	ld a, [$143]
	bit 7, a
	call z, emulate_dmg
	bit 7, a

	ldh [$4C], a
	ldh a, [title_checksum]
	ld b, a

	jr z, .skip_dmg_for_cgb_check
	ldh a, [input_palette]
	and a
	jr nz, .emulate_dmg_for_cgb_game

.skip_dmg_for_cgb_check

IF DEF(AGB)
	xor a
	ld c, a
	add a, $11
	ld h, c
	ld b, 1
ELSE
	xor a
	ld c, a
	ld a, $11
	ld h, c
ENDC
	ret

.emulate_dmg_for_cgb_game
	call emulate_dmg
	ldh [$4C], a
	ld a, $1
	ret

emulate_dmg:
	ld a, 1
	ldh [$6C], a
	call get_palette_index
	bit 7, a
	call nz, load_dmg_tilemap
	and $7F
	ld b, a
	ldh a, [input_palette]
	and a
	jr z, .nothing_down
	ld hl, key_combo_palettes - 1
	ld c, a
	ld b, 0
	add hl, bc
	ld a, [hl]
	jr .palette_from_keys
.nothing_down
	ld a, b
.palette_from_keys
	call wait_frame
	call load_palettes_from_index
	ld a, 4
	ld d, 0
	ld e, $8
	ld l, $7C
	ret

get_palette_index:
	ld hl, $14B
	ld a, [hl]
	cp $33
	jr z, .new_licensee
	dec a
	jr nz, .not_nintendo
	jr .do_checksum
.new_licensee
	ld l, $44
	ld a, [hli]
	cp "0"
	jr nz, .not_nintendo
	ld a, [hl]
	cp "1"
	jr nz, .not_nintendo

.do_checksum:
	ld l, $34
	ld c, $10
	xor a

.checksum_loop:
	add [hl]
	inc l
	dec c
	jr nz, .checksum_loop
	ld b, a

	ld hl, title_checksums

.search_loop:
	ld a, l
	sub LOW(checksums_end)
	ret z
	ld a, [hli]
	cp b
	jr nz, .search_loop

	ld a, l
	sub first_checksum_with_duplicate - title_checksums
	jr c, .match
	push hl
	ld a, l
	add dups_4th_letter_array - first_checksum_with_duplicate - 1
	ld l, a
	ld a, [hl]
	pop hl
	ld c, a
	ld a, [$134 + 3]
	cp c
	jr nz, .search_loop

.match:
	ld a, l
	add palette_per_checksum - title_checksums - 1
	ld l, a
	ld a, b
	ldh [title_checksum], a
	ld a, [hl]
	ret

.not_nintendo:
	xor a
	ret

load_palettes_from_index:
	ld b, a
	add b
	add b
	ld hl, palette_combinations
	ld b, 0
	ld c, a
	add hl, bc

	ld e, 0

.load_obj_palette:
	ld a, [hli]
	push hl
	ld hl, palettes
	ld b, 0
	ld c, a
	add hl, bc
	ld d, 8
	call load_obj_palettes
	pop hl
	bit 3, e
	jr nz, .load_bg_palette
	ld e, 8
	jr .load_obj_palette
.load_bg_palette
	ld a, [hli]
	ld hl, palettes
	ld b, 0
	ld c, a
	add hl, bc
	ld d, 8
	jp load_bg_palettes

brighten_color:
	ld a, [hli]
	ld e, a
	ld a, [hld]
	ld d, a
	ld bc, $421

	ld a, e
	and $1F
	cp $1F
	jr nz, .blue_not_maxed
	res 0, c
.blue_not_maxed
	
	ld a, e
	and $E0
	cp $E0
	jr nz, .green_not_maxed
	ld a, d
	and $3
	cp $3
	jr nz, .green_not_maxed
	res 5, c
.green_not_maxed

	ld a, d
	and $7C
	cp $7C
	jr nz, .red_not_maxed
	res 2, b

.red_not_maxed

	push hl
	ld h, d
	ld l, e
	add hl, bc
	ld d, h
	ld e, l
	pop hl

	ld a, e
	ld [hli], a
	ld a, d
	ld [hli], a
	ret

clear_vram_via_hdma:
	ld hl, $FF51

	ld a, $88
	ld [hli], a
	xor a
	ld [hli], a

	ld a, $98
	ld [hli], a
	ld a, $A0
	ld [hli], a
	ld [hl], $12
	ret

get_input_palette_index:
	ld a, $20
	ldh [$00], a
	ldh a, [$00]
	cpl
	and $F
	ret z
	push bc
	ld c, 0

.direction_loop
	inc c
	rra
	jr nc, .direction_loop
	
	ld a, $10
	ldh [$00], a
	ldh a, [$00]
	cpl
	rla
	rla
	and $C
	add c
	ld b, a
	ldh a, [input_palette]
	ld c, a
	ld a, b
	ldh [input_palette], a
	cp c
	pop bc
	ret z

change_animation_palette:
	push af
	push hl
	push bc
	push de
	ld hl, key_combo_palettes - 1
	ld c, a
	ld b, 0	
	add hl, bc
	ld a, [hl]
	ld b, a
	add b
	add b

	ld hl, palette_combinations + 2
	ld b, 0
	ld c, a
	add hl, bc
	ld a, [hl]
	ld hl, palettes + 1
	ld b, 0	
	ld c, a
	add hl, bc
	ld a, [hld]
	cp $7F
	jr nz, .is_white
	inc hl
	inc hl

.is_white
	push af
	ld a, [hli]
	push hl
	ld hl, bg_palettes
	call replace_color_in_all_palettes
	pop hl
	ldh [bg_palettes + 2], a
	
	ld a, [hli]
	push hl

	ld hl, bg_palettes + 1
	call replace_color_in_all_palettes
	pop hl
	ldh [bg_palettes + 3], a
	pop af
	jr z, .is_not_white
	inc hl
	inc hl

.is_not_white:
	ld a, [hli]
	ldh [bg_palettes + 7 * 8 + 2], a
	ld a, [hli]
	ldh [bg_palettes + 7 * 8 + 3], a
	ld a, [hli]
	ldh [bg_palettes + 4], a
	ld a, [hl]
	ldh [bg_palettes + 5], a

	call wait_frame
	ld hl, bg_palettes
	call load_bg_palettes_64
	ld a, 30
	ldh [wait_loop_counter], a
	pop de
	pop bc
	pop hl
	pop af
	ret

replace_color_in_all_palettes:
	ld de, 8
	ld c, 8
.loop
	ld [hl], a
	add hl, de
	dec c
	jr nz, .loop
	ret

load_dmg_tilemap:
	push af
	call wait_frame
	ld a, $19
	ld [$9910], a
	ld hl, $992F
	ld c, $C

.tilemap_loop
	dec a
	jr z, .tilemap_done
	ldd [hl], a
	dec c
	jr nz, .tilemap_loop
	ld l, $0F
	jr .tilemap_loop

.tilemap_done
	pop af
	ret

init_waveform:
	ld hl, $FF30

	xor a
	ld c, $10

.waveform_loop
	ldi [hl], a
	cpl
	dec c
	jr nz, .waveform_loop
	ret

SECTION "RomMAX", ROM0[$900]
	ds 1

SECTION "HRAM" , HRAM[$FF80]

title_checksum:
	ds 1
bg_palettes:
	ds 8 * 4 * 2
input_palette:
	ds 1
wait_loop_counter:
	ds 1