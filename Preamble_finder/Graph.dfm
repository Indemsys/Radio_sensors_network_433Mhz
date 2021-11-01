object frmGraph: TfrmGraph
  Left = 159
  Top = 90
  Width = 383
  Height = 265
  Caption = 'График автокорреляции'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Chart1: TChart
    Left = 0
    Top = 33
    Width = 375
    Height = 205
    Title.Text.Strings = (
      'TChart')
    Title.Visible = False
    View3D = False
    Align = alClient
    BevelOuter = bvNone
    Color = clWhite
    TabOrder = 0
    object Series1: TPointSeries
      Marks.ArrowLength = 0
      Marks.Visible = False
      PercentFormat = '##0.##,%'
      SeriesColor = clRed
      ShowInLegend = False
      ClickableLine = False
      Pointer.HorizSize = 3
      Pointer.InflateMargins = True
      Pointer.Pen.Color = clBlue
      Pointer.Style = psCircle
      Pointer.VertSize = 3
      Pointer.Visible = True
      XValues.DateTime = False
      XValues.Name = 'X'
      XValues.Order = loAscending
      YValues.DateTime = False
      YValues.Name = 'Y'
      YValues.Order = loNone
    end
  end
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 375
    Height = 33
    Align = alTop
    TabOrder = 1
    object TeeCommander1: TTeeCommander
      Left = 1
      Top = 1
      Width = 373
      Height = 31
      Panel = Chart1
      Align = alClient
      ParentShowHint = False
      TabOrder = 0
    end
  end
  object FormStorage1: TFormStorage
    StoredValues = <>
    Left = 212
    Top = 176
  end
end
