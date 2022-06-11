/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "include/threads/vaddr.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void vm_init(void)
{
	vm_anon_init();
	vm_file_init();
#ifdef EFILESYS /* For project 4 */
	pagecache_init();
#endif
	register_inspect_intr();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type(struct page *page)
{
	int ty = VM_TYPE(page->operations->type);
	switch (ty)
	{
	case VM_UNINIT:
		return VM_TYPE(page->uninit.type);
	default:
		return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim(void);
static bool vm_do_claim_page(struct page *page);
static struct frame *vm_evict_frame(void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool vm_alloc_page_with_initializer(enum vm_type type, void *upage, bool writable,
									vm_initializer *init, void *aux)
{

	ASSERT(VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page(spt, upage) == NULL)
	{
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */

		/* TODO: Insert the page into the spt. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page(struct supplemental_page_table *spt UNUSED, void *va UNUSED)
{
	struct page *page = NULL;
	/* TODO: Fill this function. */
	/* pg_round_down()으로 vaddr의 페이지 번호를 얻음 */
	page = pg_round_down(va); /* 수상함 - vaddr 주소 타입으로 받아야 함? */
	/* hash_find() 함수를 사용해서 hash_elem 구조체 얻음 */
	struct hash_elem *page_hash_elem = hash_find(&spt->vm, &page->hash_elem);
	/* 만약 존재하지 않는다면 NULL 리턴 */
	if (page_hash_elem)
	{
		/* hash_entry()로 해당 hash_elem의 vm_entry 구조체 리턴 */
		page = hash_entry(page_hash_elem, struct page, hash_elem);
		return page;
	}
	return NULL;
}

/* Insert PAGE into spt with validation. */
bool spt_insert_page(struct supplemental_page_table *spt UNUSED,
					 struct page *page UNUSED)
{
	int succ = false;
	if (!hash_insert(&spt->vm, &page->hash_elem))
	{
		succ = true;
	}
	return succ;
}

void spt_remove_page(struct supplemental_page_table *spt, struct page *page)
{
	vm_dealloc_page(page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim(void)
{
	struct frame *victim = NULL;
	/* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame(void)
{
	struct frame *victim UNUSED = vm_get_victim();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
/* palloc()을 수행하고 frame을 얻는다.
 * 만약 가용한 페이지가 없으면 페이지를 제거하고 반환한다.
 * 해당 함수는 항상 valid한 주소를 반환해야한다.
 * (user pool 메모리가 가득찬 경우,
 * 프레임을 제거해서 가용한 메모리 공간을 확보해야한다.) */
static struct frame *
vm_get_frame(void)
{
	struct page *p_page = palloc_get_page(PAL_USER);
	if (!p_page)
	{
		/* 만약 가용한 페이지가 없으면 페이지를 제거하고 반환한다. */
		palloc_free_page((void *)p_page);
		/* 할당 실패 시 return 어떻게 할지 보기 */
	}

	/* 안쪽에서는 결국 memset
	근데 size of (struct frame) 만큼 할당하지 않아도 되는건가 */
	struct frame *frame = palloc_get_page(PAL_USER);
	if (frame)
	{
		/* frame member들을 초기화 */
		frame = p_page->frame; /* 수상함 - 이거 vm_do_claim_page에서 해주나? */
		frame->kva = p_page;   /* 승준 오빠 슬랙 참고 */

		ASSERT(frame != NULL);
		ASSERT(frame->page == NULL);

		return frame;
	}
	/* frame 할당 실패 시엔 return 어떻게? */
}

/* Growing the stack. */
static void
vm_stack_growth(void *addr UNUSED)
{
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp(struct page *page UNUSED)
{
}

/* Return true on success */
bool vm_try_handle_fault(struct intr_frame *f UNUSED, void *addr UNUSED,
						 bool user UNUSED, bool write UNUSED, bool not_present UNUSED)
{
	struct supplemental_page_table *spt UNUSED = &thread_current()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */

	return vm_do_claim_page(page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void vm_dealloc_page(struct page *page)
{
	destroy(page);
	free(page);
}

/* Claim the page that allocate on VA. */
/* VA에 할당되어있는 페이지를 요청한다. */
bool vm_claim_page(void *va UNUSED)
{
	struct page *page = palloc_get_page(PAL_USER);

	page->vaddr = va;

	return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
/* 페이지를 요청하고 MMU를 세팅한다. */
static bool
vm_do_claim_page(struct page *page)
{
	struct frame *frame = vm_get_frame();

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO
	 * : 페이지의 VA를 프레임의 PA에 매핑하기 위해
	 *   PTE insert */
	/* mmu.c 함수 일단 사용해봄.
	 * user page = page & kernel page = frame이 맞는건지 모르겠음 */
	pml4_set_page(thread_current()->pml4, page, frame, page->writable);

	return swap_in(page, frame->kva);
}

/* Initialize new supplemental page table */
void supplemental_page_table_init(struct supplemental_page_table *spt UNUSED)
{
	hash_init(&spt->vm, page_hash, page_less, NULL);
}

/* Copy supplemental page table from src to dst */
bool supplemental_page_table_copy(struct supplemental_page_table *dst UNUSED,
								  struct supplemental_page_table *src UNUSED)
{
}

/* Free the resource hold by the supplemental page table */
void supplemental_page_table_kill(struct supplemental_page_table *spt UNUSED)
{
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}
