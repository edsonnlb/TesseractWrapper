using System;
using System.Drawing;
using System.Windows.Forms;
using tesseract;

namespace TesseractWrapper
{
	public class TesseractEngine
	{
		private TesseractProcessor tp;

		private string lang = "eng";

		private string langFilesPath = "";

		public TesseractEngine(string langFilesPath, string lang)
		{
			this.tp = new TesseractProcessor();
			this.lang = lang;
			this.langFilesPath = langFilesPath;
			if (this.langFilesPath.get_Length() == 0)
			{
				this.langFilesPath = string.Concat(Application.get_StartupPath(), "\\tessdata\\");
			}
			if (!this.langFilesPath.EndsWith("\\"))
			{
				TesseractEngine tesseractEngine = this;
				tesseractEngine.langFilesPath = string.Concat(tesseractEngine.langFilesPath, "\\");
			}
		}

		public void End()
		{
			this.tp.End();
		}

		public bool Init()
		{
			return this.tp.Init(this.langFilesPath, this.lang, 3);
		}

		public string Recognize(string filePath, OCROutputType outputType)
		{
			string str;
			try
			{
				if (outputType != OCROutputType.HOCR)
				{
					this.tp.SetVariable("tessedit_create_hocr", "false");
				}
				else
				{
					this.tp.SetVariable("tessedit_create_hocr", "true");
				}
				str = this.tp.Apply(filePath);
			}
			catch
			{
				return "";
			}
			return str;
		}

		public string Recognize(Image img, OCROutputType outputType)
		{
			string str;
			try
			{
				string str1 = "";
				if (img.get_PixelFormat() != 196865)
				{
					str1 = (outputType != OCROutputType.HOCR ? this.tp.Apply(img, false) : this.tp.Apply(img, true));
				}
				else
				{
					Bitmap bitmap = new Bitmap(img.get_Width(), img.get_Height(), 139273);
					bitmap.SetResolution(img.get_HorizontalResolution(), img.get_VerticalResolution());
					Graphics graphic = Graphics.FromImage(bitmap);
					graphic.DrawImageUnscaled(img, 0, 0);
					graphic.Dispose();
					str1 = (outputType != OCROutputType.HOCR ? this.tp.Apply(bitmap, false) : this.tp.Apply(bitmap, true));
					bitmap.Dispose();
				}
				str = str1;
			}
			catch
			{
				return "";
			}
			return str;
		}
	}
}