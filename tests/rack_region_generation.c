/* Execute the production rack and Mach VM implementation with a paused reader. */
#include <assert.h>
#include <stdio.h>

/* magazine_testing.h uses darwintest assertions only in its setup helper. */
#define T_QUIET
#define T_ASSERT_NOTNULL(value, ...) assert((value) != NULL)
#include "magazine_testing.h"

int main(void)
{
	rack_t rack = {0};
	rack_init(&rack, RACK_TYPE_TINY, 1, 0);
	/* Region indexing never dereferences these synthetic aligned addresses. */
	region_t anchor = (region_t)((uintptr_t)1 << HASH_BLOCKS_ALIGN);
	rack_region_insert(&rack, anchor);
	region_hash_generation_t *saved = os_atomic_load(&rack.region_generation, acquire);
	region_t *table = saved->hashed_regions;

	/* Suspend the reader after taking its table, before reading its geometry. */
	for (size_t i = 2; i <= 2 * INITIAL_NUM_REGIONS; ++i) {
		rack_region_insert(&rack, (region_t)((uintptr_t)i << HASH_BLOCKS_ALIGN));
	}
	rgnhdl_t slot = hash_lookup_region_no_lock(table, saved->num_regions_allocated,
			saved->num_regions_allocated_shift, anchor);
	int found = slot && *slot == anchor;
	printf("RACK_REGION_GENERATION_%s retained_lookup=%d\n", found ? "OK" : "LOST", found);
	rack_destroy(&rack);
	return found ? 0 : 1;
}
