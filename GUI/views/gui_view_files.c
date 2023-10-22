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

#define DIR_PATH_LABEL_OFFSET_X 12
#define DIR_PATH_LABEL_OFFSET_Y 3
#define DIR_PATH_LABEL_WIDTH (GUI_TAB_WIDTH - 2 * DIR_PATH_LABEL_OFFSET_X)
#define DIR_PATH_LABEL_HEIGHT 16
#define LIST_HEIGHT (GUI_SCREEN_HEIGHT - (DIR_PATH_LABEL_HEIGHT + DIR_PATH_LABEL_OFFSET_Y))

struct gui_files_ctx_t
{
	lv_obj_t *tab_files;
	lv_obj_t *dir_path_label;
	lv_obj_t *list_files;
	dir_list_t *dirs_shown;
	dir_list_t *dirs_played;
	void (*on_click)(const char *fs_path, dir_entry_t *entry, dir_list_t *dir_list);
};

static struct gui_files_ctx_t gui_files_ctx;

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
	memset(&gui_files_ctx, 0, sizeof(gui_files_ctx));

	/* Set initial directory */
	dir_init(GUI_VIEW_FILES_INITIAL_PATH);

	/* Create files tab */
	gui_files_ctx.tab_files = lv_tabview_add_tab(sidebar, LV_SYMBOL_FILE);
	lv_obj_set_style_pad_left(gui_files_ctx.tab_files, 0, 0);
	lv_obj_set_style_pad_top(gui_files_ctx.tab_files, 0, 0);
	lv_obj_clear_flag(gui_files_ctx.tab_files, LV_OBJ_FLAG_SCROLLABLE);

	/* Create directory path label */
	gui_files_ctx.dir_path_label = lv_label_create(gui_files_ctx.tab_files);
	lv_label_set_long_mode(gui_files_ctx.dir_path_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
	lv_obj_set_size(gui_files_ctx.dir_path_label, DIR_PATH_LABEL_WIDTH, DIR_PATH_LABEL_HEIGHT);
	lv_obj_align(gui_files_ctx.dir_path_label, LV_ALIGN_TOP_LEFT, DIR_PATH_LABEL_OFFSET_X, DIR_PATH_LABEL_OFFSET_Y);

	/* Create files list */
	gui_files_ctx.list_files = lv_list_create(gui_files_ctx.tab_files);
	lv_obj_set_size(gui_files_ctx.list_files, GUI_TAB_WIDTH, LIST_HEIGHT);
	lv_obj_set_style_radius(gui_files_ctx.list_files, GUI_BASE_CORNER_RADIUS, LV_PART_MAIN);
	lv_obj_align_to(gui_files_ctx.list_files, gui_files_ctx.dir_path_label, LV_ALIGN_OUT_BOTTOM_LEFT, -DIR_PATH_LABEL_OFFSET_X, 0);

	reload_list();
}

void gui_view_files_set_on_click_callback(void (*on_click)(const char *fs_path, dir_entry_t *entry, dir_list_t *dir_list))
{
	gui_files_ctx.on_click = on_click;
}

/* Private functions */
static bool is_directory(const FILINFO *fno)
{
	return (fno->fattrib & AM_DIR);
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
	/* If currently played directory is different than currently shown, destroy played dir list and update it */
	if (gui_files_ctx.dirs_played != gui_files_ctx.dirs_shown) {
		dir_list_free(gui_files_ctx.dirs_played);
		gui_files_ctx.dirs_played = gui_files_ctx.dirs_shown;
	}

	const char *filename = (const char *)lv_event_get_user_data(event);
	dir_entry_t *current_entry = dir_find_entry(gui_files_ctx.dirs_played, filename);
	if (gui_files_ctx.on_click != NULL) {
		gui_files_ctx.on_click(dir_get_fs_path(), current_entry, gui_files_ctx.dirs_played);
	}
}

static void on_unsupported_file_click(lv_event_t *event)
{
	const char *filename = (const char *)lv_event_get_user_data(event);
	gui_popup_unsupported_create(filename);
}

static void reload_list()
{
	/* Set directory path label */
	lv_label_set_text(gui_files_ctx.dir_path_label, dir_get_fs_path());

	/* Delete list if not used in playback */
	if (gui_files_ctx.dirs_played != gui_files_ctx.dirs_shown) {
		dir_list_free(gui_files_ctx.dirs_shown);
	}

	/* Delete current list contents */
	lv_obj_clean(gui_files_ctx.list_files);

	/* Create new files list */
	gui_files_ctx.dirs_shown = dir_list();

	/* Create directory up button */
	lv_obj_t *button;
	if (!dir_is_top()) {
		button = lv_list_add_btn(gui_files_ctx.list_files, NULL, "..");
		lv_obj_add_event_cb(button, on_directory_up_click, LV_EVENT_CLICKED, NULL);
	}

	dir_entry_t *const first_dir = gui_files_ctx.dirs_shown->head;
	if (first_dir == NULL) {
		return;
	}

	dir_entry_t *current_dir = first_dir;
	do {
		const FILINFO *fno = dir_get_fd(current_dir);

		if (is_directory(fno)) {
			button = lv_list_add_btn(gui_files_ctx.list_files, LV_SYMBOL_DIRECTORY, fno->fname);
			lv_obj_add_event_cb(button, on_directory_click, LV_EVENT_CLICKED, (void *)fno->fname);
		}
		else if (is_supported(fno)) {
			button = lv_list_add_btn(gui_files_ctx.list_files, LV_SYMBOL_AUDIO, fno->fname);
			lv_obj_add_event_cb(button, on_supported_file_click, LV_EVENT_CLICKED, (void *)fno->fname);
		}
		else {
			button = lv_list_add_btn(gui_files_ctx.list_files, LV_SYMBOL_FILE, fno->fname);
			lv_obj_add_event_cb(button, on_unsupported_file_click, LV_EVENT_CLICKED, (void *)fno->fname);
		}

		current_dir = dir_get_next(gui_files_ctx.dirs_shown, current_dir);
	} while (current_dir != first_dir);
}
