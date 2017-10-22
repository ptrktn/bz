/****h* bzsim/jpg.c
 *  NAME
 *    jpg.c
 *  DESCRIPTION
 *    See example.c in jpeg source code for detailed explanation 
 *********/

#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <string.h>
#include "bzsim.h"

typedef struct my_error_mgr *my_error_ptr;

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf         setjmp_buffer;	/* for return to caller */
};


/****f* jpgutils.c/my_error_exit
 *  NAME
 *    my_error_exit
 *  SYNOPSIS
 *    METHODDEF(void) my_error_exit(j_common_ptr cinfo)
 *  SOURCE
 */

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr    myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

/************ my_error_exit */

/****f* jpg.c/write_JPEG_file
 *  NAME
 *    write_JPEG_file
 *  SYNOPSIS
 *    static int write_JPEG_file
 *  SOURCE
 */
static int      write_JPEG_file(
				                char *filename,
				                JSAMPLE * image_buffer,	/* Points to large array
									 * of R,G,B-order data */
				                int image_height,	/* Number of rows in
									 * image */
				                int image_width,	/* Number of columns in
									 * image */
				                int ncomponents,	/* 1 = grayscale, 3 =
									 * colour */
				                int quality	/* 1 (worst) ... 100
								 * (best) */
)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE           *outfile;/* target file */
	JSAMPROW        row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int             row_stride;	/* physical row width in image buffer */

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	if ((outfile = fopen(filename, "wb")) == NULL) {
		fprintf(stderr, "write_JPEG_file: can not open '%s' for writing\n", filename);
		return BZSIM_ERROR;
	}
	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = image_width;	/* image width and height, in
						 * pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = ncomponents;	/* # of color components per
						 * pixel */
	cinfo.in_color_space = (ncomponents == BZSIM_BW ? JCS_GRAYSCALE : JCS_RGB);
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE	/* limit to baseline-JPEG
			     values */ );
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = image_width * ncomponents;	/* JSAMPLEs per row in
						 * image_buffer */
	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);
	return BZSIM_OK;
}

/************ write_JPEG_file */

/****f* jpg.c/bzsimWriteJPEG
 *  NAME
 *    bzsimWriteJPEG
 *  SYNOPSIS
 *    bzsimWriteJPEG(char *fname, unsigned char *pix_data, int nx, int ny, int depth, int quality)
 *  SOURCE
 */

int             bzsimWriteJPEG(char *fname, unsigned char *pix_data, int nx, int ny, int depth, int quality)
{
	assert((depth == BZSIM_BW || depth == BZSIM_COLOUR));
	return write_JPEG_file(fname, (JSAMPLE *) pix_data, ny, nx, depth, quality);
}

/************ bzsimWriteJPEG */

/****f* jpg.c/bzsimReadJPEG
 *  NAME
 *    bzsimReadJPEG
 *  SYNOPSIS
 *    unsigned char * bzsimReadJPEG(char *filename, int *nx, int *ny, int *nz)
 *  SOURCE
 */
unsigned char * bzsimReadJPEG(char *filename, int *nx, int *ny, int *nz)
{
	unsigned char  *buf = NULL;
	int             n = 0;
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE           *infile;
	JSAMPARRAY      buffer;
	int             row_stride;
	int             column_stride;

	if ((infile = fopen(filename, "rb")) == NULL)
		bzsimPanic("can not open '%s'\n", filename);

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return (unsigned char *) NULL;
	}
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);
	*nx = cinfo.output_width;
	*ny = cinfo.output_height;
	*nz = cinfo.output_components;
	/*dbg("%s dimensions: %d x %d x %d\n", filename, *nx, *ny, *nz);*/
	row_stride = cinfo.output_width * cinfo.output_components;
	column_stride = cinfo.output_height;
	buf = XALLOC(row_stride * column_stride, unsigned char);
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) & cinfo, JPOOL_IMAGE, row_stride, 1);
	while (cinfo.output_scanline < cinfo.output_height) {
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(&buf[n * row_stride], buffer[0], row_stride);
		n++;
	}
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	/* jerr.pub.num_warnings != 0 ??? */
	return buf;
}

/************ readJpg */

/****f* jpg.c/main
 *  NAME
 *    main
 *  SYNOPSIS
 *    main(int argc, char **argv)
 *  SOURCE
 */

#ifdef _TEST_
int             main(int argc, char **argv)
{
#define NX 256
#define NY 256
	unsigned char   data[NX * NY];
	int             i = 0, x, y;
	for (x = 0; x != NX; x++) {
		for (y = 0; y != NY; y++) {
			data[i++] = y;
		}
	}
	bzsimWriteJPEG("test.jpg", data, NX, NY, BZSIM_BW, 100);
	return i;
}
#endif

/************ main */
