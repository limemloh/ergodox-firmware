/* ----------------------------------------------------------------------------
 * ergoDOX : matrix specific exports
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012 Ben Blazak <benblazak.dev@gmail.com>
 * Released under The MIT License (MIT) (see "license.md")
 * Project located at <https://github.com/benblazak/ergodox-firmware>
 * ------------------------------------------------------------------------- */


#ifndef KEYBOARD__DACTYL__MATRIX_h
	#define KEYBOARD__DACTYL__MATRIX_h

	// --------------------------------------------------------------------

	#define KB_ROWS      6  // must match real life
	#define KB_COLUMNS  12  // must match real life

	// --------------------------------------------------------------------

	/* mapping from spatial position to matrix position
	 * - spatial position: where the key is spatially, relative to other
	 *   keys both on the keyboard and in the layout
	 * - matrix position: the coordinate in the matrix to which a key is
	 *   scanned by the update functions
	 *
	 * - location numbers are in the format `row##column`, where both 'row'
	 *   and 'column' are single digit hex numbers corresponding to the
	 *   matrix position (which also corresponds to the row and column pin
	 *   labels used in the teensy and mcp23018 files)
	 *
	 * - coordinates
	 *   - optional keys
	 *     k15, k16 (left hand thumb group)
	 *     k17, k18 (right hand thumb group)
	 *   - unused keys
	 *     k36, k00 (left hand)
	 *     k37, k0D (right hand)
	 *
	 * --- other info -----------------------------------------------------
	 *           rows x columns = positions;  used,  unused
	 * per hand:    6 x  7      = 42;         40,    2
	 *    total:    6 x 14      = 84;         80,    4
	 *
	 * left hand  : rows 0..5, cols 0..6
	 * right hand : rows 0..5, cols 7..D
	 * --------------------------------------------------------------------
	 */
	#define KB_MATRIX_LAYER(					\
			/* for unused positions */			\
			na,						\
									\
			/* left hand, spatial positions */		\
			k50,k51,k52,k53,k54,k55,    			\
			k40,k41,k42,k43,k44,k45,    			\
			k30,k31,k32,k33,k34,k35,			\
			k20,k21,k22,k23,k24,k25,    			\
			k10,k11,k12,k13,k14,				\
			                        k05,k04,		\
			                            k03,		\
					    k00,k01,k02,		\
									\
			/* right hand, spatial positions */		\
			        k56,k57,k58,k59,k5A,k5B,		\
			        k46,k47,k48,k49,k4A,k4B,		\
			        k36,k37,k38,k39,k3A,k3B,		\
			        k26,k27,k28,k29,k2A,k2B,		\
			            k17,k18,k19,k1A,k1B,		\
			k07,k06,					\
			k08,     					\
			k09,k0A,k0B )					\
									\
	/* matrix positions */						\
	{{ k00,k01,k02,k03,k04,k05, k06,k07,k08,k09,k0A,k0B },	\
	 { k10,k11,k12,k13,k14,na , na ,k17,k18,k19,k1A,k1B },	\
	 { k20,k21,k22,k23,k24,k25, k26,k27,k28,k29,k2A,k2B },	\
	 { k30,k31,k32,k33,k34,k35, k36,k37,k38,k39,k3A,k3B },	\
	 { k40,k41,k42,k43,k44,k45, k46,k47,k48,k49,k4A,k4B },	\
	 { k50,k51,k52,k53,k54,k55, k56,k57,k58,k59,k5A,k5B }}


	#define KB_MATRIX_LAYER_SET_ALL(na, kxx)		\
		LAYER(						\
			na,					\
								\
			kxx,kxx,kxx,kxx,kxx,kxx,		\
			kxx,kxx,kxx,kxx,kxx,kxx,		\
			kxx,kxx,kxx,kxx,kxx,kxx,		\
			kxx,kxx,kxx,kxx,kxx,kxx,		\
			kxx,kxx,kxx,kxx,kxx,			\
			                        kxx,kxx,	\
			                            kxx,	\
			                    kxx,kxx,kxx,	\
								\
			        kxx,kxx,kxx,kxx,kxx,kxx,	\
			        kxx,kxx,kxx,kxx,kxx,kxx,	\
			        kxx,kxx,kxx,kxx,kxx,kxx,	\
			        kxx,kxx,kxx,kxx,kxx,kxx,	\
			            kxx,kxx,kxx,kxx,kxx,	\
			kxx,kxx,				\
			kxx,        				\
			kxx,kxx,kxx )				\

#endif

