/* Clocks and broken down time, which is where glibc's largest recent rearrangement landed.
 *
 * 2.34 gave 32-bit machines a 64-bit time_t and a second spelling of every function that takes one,
 * and the header decides which spelling a call gets from _TIME_BITS. The 64-bit machines here were
 * never affected, and that is the point: this has to give the same answers on a release that has
 * the split and on a release that does not, because the tree carries both and a target asking for
 * one must not get the other.
 *
 * struct tm is the other half and is older than any of that. It has nine int fields and glibc adds
 * two more at the end under _DEFAULT_SOURCE, tm_gmtoff and tm_zone, so a program compiled with one
 * feature test macro and a library compiled with another disagree about the size of something they
 * both write into. gmtime_r writes it and the caller reads it, which is the direction that finds it.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

int main(void)
{
	struct timespec now;
	struct timespec resolution;
	struct tm parts;
	time_t epoch = 1000000000;
	char text[64];

	if (clock_gettime(CLOCK_REALTIME, &now) != 0) {
		printf("clock_gettime failed\n");
		return 1;
	}
	if (clock_getres(CLOCK_REALTIME, &resolution) != 0) {
		printf("clock_getres failed\n");
		return 1;
	}

	/* tv_nsec is the field after tv_sec and is the one a wrong time_t width moves. A nanosecond
	 * count is always under a billion, and a seconds count read out of the wrong half of a struct
	 * is not. */
	printf("seconds %d\n", now.tv_sec > 1600000000);
	printf("nanoseconds %d\n", now.tv_nsec >= 0 && now.tv_nsec < 1000000000);
	printf("resolution %d\n", resolution.tv_sec == 0 && resolution.tv_nsec > 0);

	/* A fixed instant, so every release and every machine has to say the same thing about it.
	 * 1000000000 is 2001-09-09 01:46:40 UTC, which is a Sunday. */
	memset(&parts, 0, sizeof parts);
	if (gmtime_r(&epoch, &parts) == NULL) {
		printf("gmtime_r failed\n");
		return 1;
	}
	printf("date %04d-%02d-%02d\n", parts.tm_year + 1900, parts.tm_mon + 1, parts.tm_mday);
	printf("time %02d:%02d:%02d\n", parts.tm_hour, parts.tm_min, parts.tm_sec);
	printf("weekday %d yearday %d\n", parts.tm_wday, parts.tm_yday);

	/* strftime reads the same structure from the library's side, so a field the caller wrote at
	 * one offset and the library reads at another shows up here and not above. */
	if (strftime(text, sizeof text, "%Y-%m-%dT%H:%M:%S", &parts) == 0) {
		printf("strftime failed\n");
		return 1;
	}
	printf("formatted %s\n", text);

	/* mktime the other way round, with timegm because mktime is local time and the image has no
	 * time zone database in it. */
	printf("roundtrip %d\n", timegm(&parts) == epoch);

	return 0;
}
