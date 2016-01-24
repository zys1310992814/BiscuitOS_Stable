#include "linux/kernel.h"
#include "linux/mmzone.h"
#include "linux/debug.h"
#include "linux/vmstat.h"
#include "linux/mm.h"

void __dec_zone_page_state(struct page *page,enum zone_stat_item item)
{
	__dec_zone_state(page_zone(page),item);
}
/*
 * Use interrupt disable to seralize counter updates.
 */
void mod_zone_page_state(struct zone *zone,enum zone_stat_item item,
		int delta)
{
	unsigned long flags;

	__mod_zone_page_state(zone,item,delta);
}