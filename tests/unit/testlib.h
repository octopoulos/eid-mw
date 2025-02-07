#ifndef EIDMW_TESTLIB_H
#define EIDMW_TESTLIB_H

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>

#define TEST_RV_SKIP 77		// defined by automake
#define TEST_RV_FAIL 1
#define TEST_RV_OK 0

extern int va_counter;
extern int fc_counter;

void report_failure(const char *suitename, const char *test, const char * failtype, const char * details, ...);
void report_success(const char *suitename, const char *test);
void report_skip(const char *suitename, const char * test, const char * why);
void write_xml_report(void);
#include <stdlib.h>

#ifndef TEST_NO_ABORT
#define maybe_abort() do { write_xml_report(); abort(); } while (0)
#else
#define maybe_abort() do { return TEST_RV_FAIL; } while (0)
#endif

#define verbose_assert(a) { printf("assertion %d: \"%s\": ", va_counter++, #a); if(!(a)) { report_failure(eid_testlib_funcname, #a, "AssertionFailure", "assertion %s failed", #a); fprintf(stderr, "assertion %s failed\n", #a); maybe_abort(); } else { report_success(eid_testlib_funcname, #a); printf("ok\n"); } }

#ifdef __GNUC__
#define EIDT_LIKELY(expr) __builtin_expect((expr), 1)
#define EIDT_UNLIKELY(expr) __builtin_expect((expr), 0)
#define EIDT_UNUSED __attribute__((unused))
#else
#define EIDT_LIKELY(expr) (expr)
#define EIDT_UNLIKELY(expr) (expr)
#define EIDT_UNUSED
#endif

typedef struct
{
	CK_RV rv;
	int retval;
} ckrv_mod;

static const ckrv_mod m_p11_noinit[] = {
	{CKR_OK, TEST_RV_FAIL},
	{CKR_CRYPTOKI_NOT_INITIALIZED, TEST_RV_OK},
};

static const ckrv_mod m_p11_badarg[] = {
	{CKR_OK, TEST_RV_FAIL},
	{CKR_ARGUMENTS_BAD, TEST_RV_OK},
};

static const ckrv_mod m_p11_badslot[] = {
	{CKR_OK, TEST_RV_FAIL},
	{CKR_SLOT_ID_INVALID, TEST_RV_OK},
};

static const ckrv_mod m_p11_nocard[] = {
	{CKR_OK, TEST_RV_FAIL},
	{CKR_DEVICE_ERROR, TEST_RV_OK},
	{CKR_DEVICE_REMOVED, TEST_RV_OK},
};

typedef enum {
	NOTHING_FOUND,
	VELLEMAN_FOUND,
	SYSTEM_FOUND,
} reading_pos;

#define CHECK_RV_DEALLOC
#define check_rv_late_action(func, count, mods) { \
	int retval = ckrv_decode(rv, func, count, mods); \
	if(EIDT_UNLIKELY(retval != TEST_RV_OK)) { \
		report_failure(eid_testlib_funcname, #func, "UnexpectedReturn", "unexpectedly received %d for %s", rv, #func); \
		printf("not ok\n"); \
		C_Finalize(NULL_PTR); \
		if(retval == TEST_RV_FAIL) { \
			maybe_abort(); \
		} \
		CHECK_RV_DEALLOC; \
		report_skip(eid_testlib_funcname, #func, "UnexpectedReturn"); \
		return retval; \
	} else { \
		report_success(eid_testlib_funcname, #func); \
	} \
}
#define check_rv_late(func) check_rv_late_action(func, 0, NULL)
#define check_rv_late_long(func, mods) { \
	int c = sizeof(mods) / sizeof(ckrv_mod); \
	check_rv_late_action(func, c, mods); \
}
#define check_rv_action(call, count, mods) { \
	CK_RV rv = call; \
	int retval = ckrv_decode(rv, #call, count, mods); \
	if(EIDT_UNLIKELY(retval != TEST_RV_OK)) { \
		report_failure(eid_testlib_funcname, #call, "UnexpectedReturn", "unexpectedly received %d for %s", rv, #call); \
		printf("not ok\n"); \
		C_Finalize(NULL_PTR); \
		if(retval == TEST_RV_FAIL) {\
			maybe_abort(); \
		} \
		CHECK_RV_DEALLOC; \
		report_skip(eid_testlib_funcname, #call, "UnexpectedReturn"); \
		return retval; \
	} else { \
		report_success(eid_testlib_funcname, #call); \
	} \
}
#define check_rv(call) check_rv_action(call, 0, NULL)
#define check_rv_long(call, mods) { int c = sizeof(mods) / sizeof(ckrv_mod); check_rv_action(call, c, mods); }

int ckrv_decode(CK_RV rv, char *fc, int count, const ckrv_mod *);

char *ckm_to_charp(CK_MECHANISM_TYPE);

#ifdef HAVE_CONFIG_H
#define TEST_FUNC(a) static const char * eid_testlib_funcname = #a; int main(void)
#else
#define TEST_FUNC(a) static const char * eid_testlib_funcname = #a; int a(void)
#endif

/* Verifies that a string does not contain a NULL character */
int verify_null_func(CK_UTF8CHAR * string, size_t length, int nulls_expected, char *msg, const char *eid_testlib_funcname);

#define verify_null(s, l, e, m) { int retval = verify_null_func(s, l, e, m, eid_testlib_funcname); if(EIDT_UNLIKELY(retval != TEST_RV_OK)) { report_failure(eid_testlib_funcname, "verify_null(" #s ", " #l ", " #e ", " #m ")", "not null where needed", "retval = %d", retval); return retval; } }

/* Functions to work with card moving robots */
CK_BBOOL have_robot(void);
CK_BBOOL is_manual_robot(void);
CK_BBOOL can_confirm(void);
CK_BBOOL have_pin(void);
CK_BBOOL can_enter_pin(CK_SLOT_ID slot);
CK_BBOOL open_robot(char* envvar);
CK_BBOOL open_reader_robot(char* envvar);
void robot_cmd(char cmd, CK_BBOOL check_result);
void reader_cmd(char cmd, CK_BBOOL check_result);
void robot_remove_card(void);
void robot_remove_card_delayed(void);
void robot_insert_card(void);
void robot_insert_card_delayed(void);
void robot_remove_reader(void);
void robot_remove_reader_delayed(void);
void robot_insert_reader(void);
void robot_insert_reader_delayed(void);
void hex_dump(char *data, CK_ULONG length);

/* Functions to work with reader robots */
CK_BBOOL have_reader_robot(void);
void robot_remove_reader(void);
void robot_remove_reader_delayed(void);
void robot_insert_reader(void);
void robot_remove_card_delayed(void);

enum robot_type {
	ROBOT_NONE,
	ROBOT_MECHANICAL_TURK,
	ROBOT_AUTO,
	ROBOT_AUTO_2,
};

enum dialogs_type {
	DIALOGS_AVOID,
	DIALOGS_NOPIN,
	DIALOGS_OK,
};

extern enum robot_type robot_type;
extern enum dialogs_type dialogs_type;

/* Helper functions to not have to repeat common operations all the time */
#define find_slot(with_token, slot) find_slot_func(with_token, slot, eid_testlib_funcname)
int find_slot_func(CK_BBOOL with_token, CK_SLOT_ID_PTR slot, const char * eid_testlib_funcname);

/* function definitions for tests that exist */
int scardcom_common(void);
int init_finalize(void);
int getinfo(void);
int funclist(void);
int slotlist(void);
int slotinfo(void);
int tkinfo(void);
int double_init(void);
int fork_init(void);
int slotevent(void);
int mechlist(void);
int mechinfo(void);
int sessions(void);
int sessions_nocard(void);
int sessioninfo(void);
int slogin(void);
int sbadlogin(void);
int exceptiondata(void);
int nonsensible(void);
int objects(void);
int readdata(void);
int readdata_limited(void);
int readdata_sequence(void);
int digest(void);
int threads(void);
int sign(void);
int sign_state(void);
int decode_photo(void);
int ordering(void);
int wrong_init(void);
int login_state(void);
int eject(void);
int sdialogs(void);

#endif
