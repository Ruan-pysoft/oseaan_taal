#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

#include "utils.h"

enum veranderlikheid {
	// constant/mutable in Engels
	// as iets "erfbaar" is dan word dit konstant/veranderlik gebaseer op die "buitenste" tipe
	// dit is moeilik om te verduidelik, ek sal later daaroor skryf in 'n teksdokument
	v_konstant,
	v_veranderlik,
	v_erfbaar,
};
DECL_DISPLAY_METHS_ENUM(veranderlikheid);
static const char *veranderlikheid_str[] = {
	[v_konstant] = "kon",
	[v_veranderlik] = "ver",
	[v_erfbaar] = "erf",
};

enum klas_van_tipe {
	kvt_basies,
	kvt_verwysing,
	kvt_vaste_lys,
	kvt_dinamiese_lys,
	kvt_funksie,
};
struct tipe;
struct konkrete_tipe;
enum tp_basiese_tipe {
	// die kern tipes van die taal, "primitive types" in Engels
	bt_niks, // "niks" tipe vir as 'n funksie niks terugkeer nie
	         // sy grote is nul, en daar kan nie 'n eintlike waarde van hierdie tipe bestaan nie
	         // dit word ook net in sekere kontekste toegelaat
	         // (kan bv nie 'n veranderlike van tipe "niks" hê nie)
	bt_karakter,
	bt_teks, // ekwivalent aan 'n vaste/dinamiese lys van karakters
	bt_bool, // waar of vals
	bt_nat8, // positiewe 8-bis heelgetal (natuurlike getal)
	bt_heel8, // positiewe of negatiewe 8-bis heelgetal
	bt_nat16,
	bt_heel16,
	bt_nat32,
	bt_heel32,
	bt_nat64,
	bt_heel64,
	bt_nat_masjien, // masjiengrote: 64-bis vir nou
	bt_heel_masjien,
};
DECL_DISPLAY_METHS_ENUM(tp_basiese_tipe);
static const char *tp_basiese_tipe_str[] = {
	[bt_niks] = "niks",
	[bt_karakter] = "karakter",
	[bt_teks] = "teks",
	[bt_bool] = "bool",
	[bt_nat8] = "nat8",
	[bt_heel8] = "heel8",
	[bt_nat16] = "nat16",
	[bt_heel16] = "heel16",
	[bt_nat32] = "nat32",
	[bt_heel32] = "heel32",
	[bt_nat64] = "nat64",
	[bt_heel64] = "heel64",
	[bt_nat_masjien] = "nat_masjien",
	[bt_heel_masjien] = "heel_masjien",
};
struct tp_verwysing {
	// "pointer" in Engels (alhowel "reference" dalk 'n meer akkurate terugvertaling sal wees)
	// word voorgestel in die geheue as dieselfde as bt_nat_masjien
	struct konkrete_tipe *na;
};
DECL_STD_METHS(tp_verwysing);
struct tp_vaste_lys {
	// 'n lys met 'n sekere, vaste lengte van 'n sekere tipe
	struct konkrete_tipe *van;
	size_t lengte;
};
DECL_STD_METHS(tp_vaste_lys);
struct tp_dinamiese_lys {
	// 'n lys met 'n dinamiese lengte,
	// dws die lengte kan verander word terwyl die program hardloop,
	// dus is die lengte nie bekend aan die compiler nie
	struct konkrete_tipe *van;
	// nota: as die waardes in die lys konstant is, dan is die lengte ook konstant.
	//       net so, as die waardes veranderlik is, dan is die lengte ook
};
DECL_STD_METHS(tp_dinamiese_lys);
struct tp_argumente {
	// die argumentelys van 'n funksie, die name van die argumente maak nie 'n verskil aan die tipe nie
	// 'n "dynamic array", gemaak om met nob.h te werk
	struct konkrete_tipe *items;
	size_t count;
	size_t capacity;
};
DECL_STD_METHS(tp_argumente);
struct tp_funksie {
	// 'n funksie; vir die tipe maak net die terugkeertipe en die tipes van die argumente saak
	// argumentname en die funksie se naam maak nie saam nie
	// as 'n eintlike waarde, dink ek 'n funksie is 'n lokasie in die rekenaar se geheue,
	// maar ook nie heeltemal nie, meer 'n "label".
	// Dus sal die groote van 'n funksie waarde "0" wees.
	struct konkrete_tipe *terugkeer;
	// nota: die terugkeertipe *kan* erf of kon wees,
	//       maar dit mag nie erf wees nie (net soos enige veranderlike nie erf mag wees nie)
	//       en dit mag ook nie kon wees nie
	//       (anders kan jy nie dit stel om 'n waarde terug te keer nie)
	struct tp_argumente argumente;
};
DECL_STD_METHS(tp_funksie);
struct tipe {
	enum klas_van_tipe klas;
	union {
		enum tp_basiese_tipe basies;
		struct tp_verwysing verwysing;
		struct tp_vaste_lys vaste_lys;
		struct tp_dinamiese_lys dinamiese_lys;
		struct tp_funksie funksie;
	};
};
DECL_STD_METHS(tipe);
struct konkrete_tipe {
	// 'n waarde: het nie net 'n tipe nie, maar ook 'n veranderlikheid daarby
	struct tipe tipe;
	enum veranderlikheid tipe_vlh;
};
DECL_STD_METHS(konkrete_tipe);

#endif /* TYPES_H */
