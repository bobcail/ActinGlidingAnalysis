/*
	Routine for opening Tiff files counting frames,
	Selecting a specific frame and reading
	the image data for that frame.
	DAW 12/14/04
	
*/
#ifdef LINUX
#include <tiffio.h>
#else 
#include </opt/local/include/tiffio.h>
#endif

		uint32 h, w;
		uint32 rowsperstrip;
		uint32 start=0;
		uint32 end;
		tsample_t samplesperpixel;
		int frm = 0;
		int i;
        int dircount = 0;

void TIFFReadData(TIFF* tif);
		
void read_tiff (char* filename)
{
	long flags = 0;
	int x;
	TIFF* tif;
	
	printf("Try and open the tiff file\n");	
    tif = TIFFOpen(filename, "r");
    if (tif)
	{
		tiff++;
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
		if(samplesperpixel>1)
			{
			printf("I'm Sorry, I can only process B&W image data!\n");
			TIFFClose(tif);
			exit (1);
			}		

        do {
            dircount++;
			} while (TIFFReadDirectory(tif));

		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		printf("There are %d X %d pixels/frame and %d frames in %s\n", w, h, dircount, filename);
				nnx=w;
				nny=h;
				nnz=dircount;
		do
			{
			printf("Select Frame for Setting Threshhold: ");
			scanf("%d", &frm);
			if (frm <= 0) frm = 1;
			if (frm > dircount) printf("Please select a frame # between 1 and %d!\n",dircount); 
			}while (frm>dircount);

		frm--;
		
		if (TIFFSetDirectory(tif, frm)) TIFFReadData(tif);

		TIFFClose(tif);
   	}
}

static void
StripImage(unsigned char* pp, uint32 nrow, tsize_t scanline)
{
	register tsize_t xx;
	uint32 yy;
	int p;
	
	end = start + nrow;
        for (yy=start; yy<end; yy++)
		{
        /* copy along an x segment and then jump to the next row */
			for (xx = 0; xx < scanline; xx++) {            
				p = *pp++;
				orig[yy][xx] = p;
				}
		}
	start += nrow;
}

void
TIFFReadContigStripData(TIFF* tif)
{
	unsigned char *buf;
	tsize_t scanline = TIFFScanlineSize(tif);

	buf = (unsigned char *)_TIFFmalloc(TIFFStripSize(tif));
	if (buf) {
		uint32 row, h;
		uint32 rowsperstrip = (uint32)-1;

		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		for (row = 0; row < h; row += rowsperstrip) {
			uint32 nrow = (row+rowsperstrip > h ?
			    h-row : rowsperstrip);
			tstrip_t strip = TIFFComputeStrip(tif, row, 0);
			if (TIFFReadEncodedStrip(tif, strip, buf, nrow*scanline) < 0) break;
			StripImage(buf, nrow, scanline);
		}
		_TIFFfree(buf);
	}
}

void
TIFFReadSeparateStripData(TIFF* tif)
{
	unsigned char *buf;
	tsize_t scanline = TIFFScanlineSize(tif);

	buf = (unsigned char *)_TIFFmalloc(TIFFStripSize(tif));
	if (buf) {
		uint32 row, h;
		uint32 rowsperstrip = (uint32)-1;
		tsample_t s, samplesperpixel;

		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
		for (row = 0; row < h; row += rowsperstrip) {
			for (s = 0; s < samplesperpixel; s++) {
				uint32 nrow = (row+rowsperstrip > h ?
				    h-row : rowsperstrip);
				tstrip_t strip = TIFFComputeStrip(tif, row, s);
				if (TIFFReadEncodedStrip(tif, strip, buf, nrow*scanline) < 0) break;
				StripImage(buf, nrow, scanline);
			}
		}
		_TIFFfree(buf);
	}
}

void
TIFFReadData(TIFF* tif)
{
	uint16 config;

	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

		if (config == PLANARCONFIG_CONTIG)
			TIFFReadContigStripData(tif);
		else
			TIFFReadSeparateStripData(tif);
}
