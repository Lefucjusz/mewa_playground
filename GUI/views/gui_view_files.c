/*
 * gui_view_files.c
 *
 *  Created on: Oct 17, 2023
 *      Author: lefucjusz
 */

#include "gui_view_files.h"
#include "gui_popup_unsupported.h"
#include "gui_dimensions.h"
#include "utils.h"
#include "dir.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

struct gui_view_files_ctx_t
{
	lv_obj_t *tab_files;
	lv_obj_t *list_files;
	void (*on_click_callback)(const dir_entry_t *entry, const dir_list_t *dir_list);
	dir_list_t *dirs;
};

static struct gui_view_files_ctx_t gui_view_files_ctx;

/* Forward declarations */
static bool is_directory(const FILINFO *fno);
static bool is_supported(const FILINFO *fno);
static void on_directory_up_click(lv_event_t *event);
static void on_directory_click(lv_event_t *event);
static void on_supported_file_click(lv_event_t *event);
static void on_unsupported_file_click(lv_event_t *event);
static void reload_list(void);

/* Public functions */
void gui_view_files_create(lv_obj_t *sidebar)
{
	/* Initialize context */
	memset(&gui_view_files_ctx, 0, sizeof(gui_view_files_ctx));

	/* Set initial directory */
	dir_init(GUI_VIEW_FILES_INITIAL_PATH);
	gui_view_files_ctx.dirs = dir_list();

	/* Create files tab */
	gui_view_files_ctx.tab_files = lv_tabview_add_tab(sidebar, LV_SYMBOL_FILE);
	lv_obj_set_style_pad_left(gui_view_files_ctx.tab_files, 0, 0);
	lv_obj_set_style_pad_top(gui_view_files_ctx.tab_files, 0, 0);
	lv_obj_clear_flag(gui_view_files_ctx.tab_files, LV_OBJ_FLAG_SCROLLABLE);

	/* Create files list */
	gui_view_files_ctx.list_files = lv_list_create(gui_view_files_ctx.tab_files);
	lv_obj_set_size(gui_view_files_ctx.list_files, tab_width, ILI9341_HEIGHT); // TODO define as GUI_MAX_HEIGHT
	lv_obj_set_style_radius(gui_view_files_ctx.list_files, corner_radius, LV_PART_MAIN);

	reload_list();
}

void gui_view_files_set_on_click_callback(void (*on_click)(const dir_entry_t *entry, const dir_list_t *dir_list))
{
	gui_view_files_ctx.on_click_callback = on_click;
}

/* Private functions */
static bool is_directory(const FILINFO *fno)
{
	return fno->fattrib & AM_DIR;
}

static bool is_supported(const FILINFO *fno)
{
	const char *extensions[] = {".mp3", ".flac", ".wav"};
	for (size_t i = 0; i < ARRAY_SIZE(extensions); ++i) {
		if (is_extension(fno->fname, extensions[i])) {
			return true;
		}
	}
	return false;
}

static void on_directory_up_click(lv_event_t *event)
{
	const int ret = dir_return();
	if (ret != 0) {
		return;
	}
	reload_list();
}

static void on_directory_click(lv_event_t *event)
{
	const char *filename = (const char *)lv_event_get_user_data(event);
	const int ret = dir_enter(filename);
	if (ret != 0) {
		return;
	}
	reload_list();
}

static void on_supported_file_click(lv_event_t *event)
{
	if (gui_view_files_ctx.on_click_callback != NULL) {
		gui_view_files_ctx.on_click_callback(NULL, NULL);
	}
}

static void on_unsupported_file_click(lv_event_t *event)
{
	const char *filename = (const char *)lv_event_get_user_data(event);
	gui_popup_unsupported_create(filename);
}

static void reload_list()
{
	/* Delete current list contents */
	dir_list_free(gui_view_files_ctx.dirs);
	lv_obj_clean(gui_view_files_ctx.list_files);

	/* Create new files list */
	gui_view_files_ctx.dirs = dir_list();

	/* Create directory path label */
	lv_list_add_text(gui_view_files_ctx.list_files, dir_get_fs_path()); // TODO this should not move

	/* Create directory up button */
	lv_obj_t *button;
	if (!dir_is_top()) {
		button = lv_list_add_btn(gui_view_files_ctx.list_files, NULL, "..");
		lv_obj_add_event_cb(button, on_directory_up_click, LV_EVENT_CLICKED, NULL);
	}

	dir_entry_t *const first_dir = gui_view_files_ctx.dirs->head;
	if (first_dir == NULL) {
		return;
	}

	dir_entry_t *current_dir = first_dir;
	do {
		const FILINFO *fno = (FILINFO *)current_dir->data;

		if (is_directory(fno)) {
			button = lv_list_add_btn(gui_view_files_ctx.list_files, LV_SYMBOL_DIRECTORY, fno->fname);
			lv_obj_add_event_cb(button, on_directory_click, LV_EVENT_CLICKED, (void *)fno->fname);
		}
		else if (is_supported(fno)) {
			button = lv_list_add_btn(gui_view_files_ctx.list_files, LV_SYMBOL_AUDIO, fno->fname);
			lv_obj_add_event_cb(button, on_supported_file_click, LV_EVENT_CLICKED, (void *)fno->fname);
		}
		else {
			button = lv_list_add_btn(gui_view_files_ctx.list_files, LV_SYMBOL_FILE, fno->fname);
			lv_obj_add_event_cb(button, on_unsupported_file_click, LV_EVENT_CLICKED, (void *)fno->fname);
		}

		current_dir = dir_get_next(gui_view_files_ctx.dirs, current_dir);
	} while (current_dir != first_dir);
}