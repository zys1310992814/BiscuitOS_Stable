/*
 * This file is used to debug Buddy System.
 *
 * Create by Buddy.D.Zhang
 */
#include "linux/kernel.h"
#include "linux/mm.h"
#include "linux/gfp.h"
#include "linux/mmzone.h"
#include "linux/list.h"
#include "linux/mm_types.h"
#include "linux/atomic.h"

#define TEST_GFP_NUM 12
static gfp_t GFP_ARRAY[] = {
	GFP_NOWAIT,
	GFP_ATOMIC,
	GFP_NOIO,
	GFP_NOFS,
	GFP_KERNEL,
	GFP_TEMPORARY,
	GFP_USER,
	GFP_HIGHUSER,
	GFP_HIGHUSER_MOVABLE,
	GFP_IOFS,
	GFP_TRANSHUGE,
	GFP_THISNODE
};
static char *GFP_NAME[] = {
	"GFP_NOWAIT",
	"GFP_ATOMIC",
	"GFP_NOIO",
	"GFP_NOFS",
	"GFP_KERNEL",
	"GFP_TEMPORARY",
	"GFP_USER",
	"GFP_HIGHUSER",
	"GFP_HIGHUSER_MOVABLE",
	"GFP_IOFS",
	"GFP_TRANSHUGE",
	"GFP_THISNODE"
};

static char *const zone_names[MAX_NR_ZONES] = {
#ifdef CONFIG_ZONE_DMA
	"DMA",
#endif
#ifdef CONFIG_ZONE_DMA32
	"DMA32",
#endif
	"Normal",
#ifdef CONFIG_HIGHMEM
	"HighMem",
#endif
	"Movable",
};

char *MigrateName[] = {
	"MIGRATE_UNMOVABLE",
	"MIGRATE_RECLAIMABLE",
	"MIGRATE_MOVABLE",
	"MIGRATE_PCPTYPES",
};
/*
 * TestCase_Buddy_normal - debug how to use buddy system.
 */
void TestCase_Buddy_normal(void)
{
	struct page *page;

	/* Alloc a page */
	page = alloc_page(GFP_KERNEL);

	PageFlage(page,"Normal");

	/* Free a page */
	mm_debug("Test Complete....\n");
}

/*
 * TestCase_allocflags_to_migratetype - Debug convert GFP to migratetype.
 * Kernel usually get three types from allocflags,"MOVABLE","UNMOVABLE"
 * and "RECLAIMABLE".we can use "gfp & GFP_MOVABLE_MASK" to get the migrate
 * type.
 *
 * Movable migrate type is "(gfp & __GFP_MOVABLE) << 1".
 * Reclaimalbe migrate type is "(gfp & __GFP_RECLAIMABLE)".
 * Unmovalbe migrate type is "0".
 *
 * Create by Buddy.D.Zhang
 */
void TestCase_allocflags_to_migratetype(void)
{	
	int i;
	
	for(i = 0 ; i < TEST_GFP_NUM ; i++)
		mm_debug("%s->migratetype:\t\t\t%d\n",GFP_NAME[i],
				allocflags_to_migratetype(GFP_ARRAY[i]));
}

/*
 * TestCase_GFP - debug page allowed flags.
 */
void TestCase_GFP(void)
{
	/* Kernel have 23 bits for GFP_XXX in a word */
	mm_debug("__GFP_BITS_MASK:");
	binary(__GFP_BITS_MASK);
	/*
	 * GFP_BOOT_MASK.
	 * Control slab gfp mask during early boot.
	 * No wait,no IO and no FS.
	 */
	mm_debug("  GFP_BOOT_MASK:");
	binary(GFP_BOOT_MASK);
}

/*
 * TestCase_GFP_ZONE - Debug usage of GFP_XXX.
 */
void TestCase_GFP_ZONE(void)
{	
	unsigned long flags;
	int i;

	/* Get the zonenum via GFP_XXX */
	mm_debug("gfp to zonenum\n");
	for(i = 0 ; i < TEST_GFP_NUM ; i++)
		mm_debug("%s:\t\t\t%d\n",GFP_NAME[i],gfp_zone(GFP_ARRAY[i]));

	/* Get the binary of GFP_ZONEMASK */
	mm_debug("GFP_ZONEMASK:");
	binary(GFP_ZONEMASK);

	mm_debug("ZONES_SHIFT:%p\n",(void *)(unsigned long)ZONES_SHIFT);

	/* GFG_ZONE_TABLE */
	mm_debug("GFP_ZONE_TABLE:");
	binary(GFP_ZONE_TABLE);

	mm_debug("OPT_ZONE_DMA %p\n",(void *)(unsigned long)OPT_ZONE_DMA);
	mm_debug("OPT_ZONE_HIGHMEM %p\n",(void *)(unsigned long)OPT_ZONE_HIGHMEM);
	mm_debug("OPT_ZONE_DMA32 %p\n",(void *)(unsigned long)OPT_ZONE_DMA32);
	mm_debug("MAX_NR_ZONES %p\n",(void *)(unsigned long)MAX_NR_ZONES);

	/*
	 * TestCase:Test GFP_ZONE_BAD Table.
	 * GFP_ZONE_BAD is a bitmap for all combinations of __GFP_DMA,
	 * __GFP_DMA32,__GFP_HIGHMEM and __GFP_MOVABLE that are not permitted.
	 * One flags per entry starting with bit0.Bit is set if the combinations 
	 * is not allowed.
	 */
	mm_debug("GFP_ZONE_BAD:");
	binary(GFP_ZONE_BAD);
	/* case0: GFP_DMA & GFP_DMA32 */
	flags = GFP_DMA | GFP_DMA32;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case1: GFP_HIGHUSER & GFP_DMA32 */
	flags = GFP_DMA32 | GFP_HIGHUSER;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case2: GFP_HIGHUSE & GFP_DMA */
	flags = GFP_DMA | GFP_HIGHUSER;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case3: GFP_HIGHUSER & GFP_DMA32 & GFP_DMA*/
	flags = GFP_DMA | GFP_HIGHUSER | GFP_DMA32;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case4: GFP_HIGHUSER & GFP_DMA & GFP_MOVABLE */
	flags = GFP_DMA | GFP_HIGHUSER_MOVABLE;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case5: __GFP_DMA & __GFP_DMA32 & __GFP_MOVABLE*/
	flags = GFP_DMA | __GFP_MOVABLE | GFP_DMA32;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case6: GFP_HIGHUSER_MOVABLE & GFP_DMA32 */
	flags = GFP_DMA | GFP_HIGHUSER_MOVABLE;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case7: GFP_HIGHUSER_MOVABLE & GFP_DMA32 & GFP_DMA */
	flags = GFP_DMA | GFP_HIGHUSER_MOVABLE | GFP_DMA;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/* case8: GFP_HIGHUSER_MOVABLE & GFP_USER */
	flags = GFP_USER | GFP_HIGHUSER_MOVABLE;
	BUG_ON((GFP_ZONE_BAD >> (flags & GFP_ZONEMASK)) & 1);

	/*
	 * TestCase:Test GFP_ZONE_TABLE.
	 */
	mm_debug("Debug GFP_ZONE_TABLE\n");
	for(i = 0 ; i < TEST_GFP_NUM ; i++) {
		unsigned long bit = GFP_ARRAY[i] & GFP_ZONEMASK;

		mm_debug("%s \t-> zone:%s\n",GFP_NAME[i],zone_names[
				(GFP_ZONE_TABLE >> (bit * ZONES_SHIFT)) & 
				((1 << ZONES_SHIFT) - 1)]);
	}
}

extern struct zoneref *next_zones_zonelist(struct zoneref *,
		enum zone_type,nodemask_t *,struct zone **);
/*
 * TestCase_zonelist - Debug zonelist and zoneref.
 */
void TestCase_zonelist(void)
{
	struct pglist_data *pgdat = NODE_DATA(0);
	struct zone *zone;
	struct zonelist *zonelist = &pgdat->node_zonelists[0];
	struct zoneref *zrf;
	int high_zoneidx = 0;

	mm_debug("zonelist->_zonerefs[0]:%s %d\n",
			zonelist->_zonerefs[0].zone->name,
			zonelist->_zonerefs[0].zone_idx);

	mm_debug("zonelist->_zonerefs[1]:%s %d\n",
			zonelist->_zonerefs[1].zone->name,
			zonelist->_zonerefs[1].zone_idx);

	zrf = next_zones_zonelist(zonelist->_zonerefs,1,NULL,&zone);
	mm_debug("Zrf %s zone %s\n",zrf->zone->name,zone->name);
	
	zrf = next_zones_zonelist(zonelist->_zonerefs,0,NULL,&zone);
	mm_debug("Zrf %s zone %s\n",zrf->zone->name,zone->name);

	zrf = first_zones_zonelist(zonelist,1,NULL,&zone);
	mm_debug("Zrf %s zone %s\n",zrf->zone->name,zone->name);
	
	zrf = first_zones_zonelist(zonelist,1,NULL,&zone);
	mm_debug("Zrf %s zone %s\n",zrf->zone->name,zone->name);

	zrf = next_zones_zonelist(++zrf,1,NULL,&zone);
	mm_debug("zrf %s Zone %s\n",zrf->zone->name,zone->name);

	for_each_zone_zonelist_nodemask(zone,zrf,zonelist,1,NULL)
		mm_debug("ZRF %s zone %s\n",zrf->zone->name,zone->name);

	for_each_zone_zonelist(zone,zrf,zonelist,1)
		mm_debug("FF %s zone %s\n",zrf->zone->name,zone->name);
}

#define ALLOC_WMARK_MIN  WMARK_MIN
#define ALLOC_WMARK_LOW  WMARK_LOW
#define ALLOC_WMARK_HIGH WMARK_HIGH
#define ALLOC_NO_WATERMARKS 0x40

#define ALLOC_WMARK_MASK (ALLOC_NO_WATERMARKS - 1)
/*
 * TestCase_WMARK - Test watermark
 */
void TestCase_WMARK(void)
{
	int alloc_flags = ALLOC_WMARK_LOW;
	unsigned long mark;
	struct zone *zone;
	struct zoneref *zrf;
	struct zonelist *zonelist = NODE_DATA(0)->node_zonelists;
	int classzone_idx;

	first_zones_zonelist(zonelist,0,NULL,&zone);
	mark = zone->watermark[alloc_flags & ALLOC_WMARK_MASK];
	classzone_idx = zone_idx(zone);

	mm_debug("mark %p\n",(void *)mark);

	mm_debug("zone_watermark_ok(%d)\n",zone_watermark_ok(zone,0,mark,
				classzone_idx,NULL));
}

/*
 * TestCase_PCP - Debug Per-CPU Allocator.
 *
 * Note:Don't use SLUB alloctor.When you alloc memory from buddy system,
 * kernel will check PCP alloctor,if PCP has free pages and get page from
 * PCP.If PCP doesn't have free page,PCP with request more pages from 
 * Buddy Allocator.
 *
 */
void TestCase_PCP(void)
{
	struct pglist_data *pgdat;
	struct zonelist *zonelist;
	struct zoneref *zrf;
	struct zone *zone;
	struct per_cpu_pages *pcp;
	struct page *page;
	struct page *page0,*page1,*page2;
	int i;

	pgdat = NODE_DATA(0);
	zonelist = pgdat->node_zonelists;

	/* Before alloc from PCP,ckeck the list of PCP */
	mm_debug("=========Before PCP=========\n");
	for_each_zone_zonelist(zone,zrf,zonelist,1) {
		pcp = &(zone->pageset)->pcp;

		mm_debug("%s PCP\n"
				"\t count:\t%p\n"
				"\t higth:\t%p\n"
				"\t batch:\t%p\n",zone->name,
				(void *)(unsigned long)pcp->count,
				(void *)(unsigned long)pcp->high,
				(void *)(unsigned long)pcp->batch);

		for(i = 0 ; i < 3 ; i++) {
			mm_debug("%s:\n",MigrateName[i]);
			list_for_each_entry(page,&pcp->lists[i],lru)
				PageFlage(page,"Page");
		}
	}

	/* Alloc physical page that belong to different migrate type. */
	page0 = alloc_page(GFP_KERNEL);
	PageFlage(page0,"MIGRATE_UNMOVABLE");
	page1 = alloc_page(GFP_TEMPORARY);
	PageFlage(page1,"MIGRATE_RECLAIM");
	page2 = alloc_page(GFP_HIGHUSER_MOVABLE);
	PageFlage(page2,"MIGRATE_MOVABLE");

	/* Check the PCP free page list */
	mm_debug("========Current PCP========\n");
	for_each_zone_zonelist(zone,zrf,zonelist,1) {
		pcp = &(zone->pageset)->pcp;

		mm_debug("%s PCP\n"
				"\t count:\t%p\n"
				"\t higth:\t%p\n"
				"\t batch:\t%p\n",zone->name,
				(void *)(unsigned long)pcp->count,
				(void *)(unsigned long)pcp->high,
				(void *)(unsigned long)pcp->batch);

		for(i = 0 ; i < 3 ; i++) {
			mm_debug("%s:\n",MigrateName[i]);
			list_for_each_entry(page,&pcp->lists[i],lru)
				PageFlage(page,"Page");
		}
}
	__free_page(page2);
	__free_page(page1);
	__free_page(page0);

	/* Check PCP free page list after free() */
	mm_debug("=========After PCP=========\n");
	for_each_zone_zonelist(zone,zrf,zonelist,1) {
		pcp = &(zone->pageset)->pcp;

		mm_debug("%s PCP\n"
				"\t count:\t%p\n"
				"\t higth:\t%p\n"
				"\t batch:\t%p\n",zone->name,
				(void *)(unsigned long)pcp->count,
				(void *)(unsigned long)pcp->high,
				(void *)(unsigned long)pcp->batch);

		for(i = 0 ; i < 3 ; i++) {
			mm_debug("%s:\n",MigrateName[i]);
			list_for_each_entry(page,&pcp->lists[i],lru)
				PageFlage(page,"Page");
		}
	}
	/* End test */
	mm_debug("Test Complete...\n");
}

/*
 * TestCase_rmqueue_smallest()
 */
void TestCase_rmqueu_smallest(void)
{
	struct pglist_data *pgdat;
	struct zonelist *zonelist;
	struct zoneref *zrf;
	struct zone *zone;
	struct free_area *area;
	struct page *page;
	int migratetype;
	int order;
	int current_order;

	pgdat = NODE_DATA(0);
	zonelist = pgdat->node_zonelists;

	/* Check all buddy page for different zone */
	for_each_zone_zonelist(zone,zrf,zonelist,1) {
		mm_debug("%s\n",zone->name);
		for(order = 0 ; order < MAX_ORDER ; order++) {
			area = &zone->free_area[order];

			mm_debug("Order %d Free page %ld\n",order,area->nr_free);
			for(migratetype = 0 ; migratetype < 3 ; migratetype++) {
				if(list_empty(&area->free_list[migratetype]))
					continue;

				list_for_each_entry(page,&area->free_list[migratetype],lru) {
					mm_debug("%s",MigrateName[migratetype]);
					PageFlage(page,"F");
				}
			}
		}
	}

	/* Check a page that from determine zone,order and migrate */
	first_zones_zonelist(zonelist,0,NULL,&zone);

	for(order = 4 ; order < MAX_ORDER ; order++) {
		if(list_empty(&zone->free_area[order].free_list[0]))
			continue;
		else {
			page = list_entry(
					zone->free_area[order].free_list[0].next,
					struct page,lru);
			PageFlage(page,"D");
			break;
		}
	}
	mm_debug("%s\n",zone->name);

	/* Allocate 16 page from Buddy system */
	order = 4;
	migratetype = MIGRATE_MOVABLE;
	first_zones_zonelist(zonelist,0,NULL,&zone);

	mm_debug("GetZone %s\n",zone->name);
	for(current_order = order ; current_order < MAX_ORDER ; current_order++) {
		int size;

		area = &zone->free_area[current_order];
		if(list_empty(&area->free_list[migratetype]))
			continue;

		page = list_entry(area->free_list[migratetype].next,struct page,lru);
		list_del(&page->lru);
		rmv_page_order(page);
		area->nr_free--;
		
		size = 1 << current_order;

		while(order < current_order) {
			area--;
			size >>= 1;
			current_order--;
			list_add(&page[size].lru,&area->free_list[migratetype]);
			area->nr_free++;
			set_page_order(&page[size],current_order);
		}
		break;
	}
	PageFlage(page,"Buddy");
}

/*
 * TestCase_page_order - Test set_page_order() and rmv_page_order()
 *
 * When we get page from buddy,the value "_mapcount" of page will set as -1
 * and the value "private" of page will set as 0.
 * When we free a page to buddy,the value "_mapcount" of apge will set as -2
 * and the value "private" of page will set ilog2(size).
 *
 * Create by Buddy.D.Zhang
 */
void TestCase_page_order(void)
{
	struct page *page;

	page = alloc_page(GFP_KERNEL);

	PageFlage(page,"GL");

#ifdef BUDDY_DEBUG_PAGE_ORDER
	mm_debug("Before set page order %p _mapcount %d\n",
			(void *)(unsigned long)page->private,
		atomic_read(&page->_mapcount));
	set_page_order(page,0);

	mm_debug("PageBuddy %d\n",PageBuddy(page));

	mm_debug("After set page order %p _mapcount %d\n",
			(void *)(unsigned long)page->private,
			atomic_read(&page->_mapcount));

	rmv_page_order(page);

	mm_debug("PageBuddy0 %d\n",PageBuddy(page));
	__SetPageBuddy(page);
	mm_debug("PageBuddy1 %d\n",PageBuddy(page));
	__ClearPageBuddy(page);
	mm_debug("PageBuddy2 %d\n",PageBuddy(page));
#endif

	__free_page(page);
}

/*
 * TestCase_Get_Buddy_Page - Get a lot of pages from the Buddy System.
 */
void TestCase_Get_Buddy_Page(void)
{
	struct pglist_data *pgdat;
	struct zonelist *zonelist;
	struct zone *zone;
	struct free_area *area;
	struct page *page;
	int migratetype;
	int order,current_order;

	pgdat = NODE_DATA(0);
	zonelist = pgdat->node_zonelists;

	first_zones_zonelist(zonelist,0,NULL,&zone);
	migratetype = MIGRATE_RECLAIMABLE;
	order = 5;
	BuddyPageMigrate(migratetype,"Before");

	for(current_order = order ; current_order < MAX_ORDER ; current_order++) {
		unsigned long size = 1 << current_order;

		area = &zone->free_area[current_order];

		if(list_empty(&area->free_list[migratetype]))
			continue;

		page = list_entry(area->free_list[migratetype].next,
				struct page,lru);
		list_del(&page->lru);
		rmv_page_order(page);
		area->nr_free--;

		while(order < current_order) {
			area--;
			current_order--;
			size >>= 1;

			list_add(&page[size].lru,&area->free_list[migratetype]);
			set_page_order(&page[size],current_order);
			area->nr_free++;
		}

		if(!PageBuddy(page))
			goto out;
	}
	return;
out:
	BuddyPageMigrate(migratetype,"After");
	PageFlage(page,"F");
}
