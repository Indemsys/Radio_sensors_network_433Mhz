object frmMain: TfrmMain
  Left = 334
  Top = 222
  Width = 555
  Height = 294
  Caption = 'Расчет автокорреляций'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 12
    Width = 86
    Height = 13
    Caption = 'Число разрядов:'
  end
  object Label2: TLabel
    Left = 240
    Top = 8
    Width = 106
    Height = 26
    Caption = 'Показывать если '#13#10'корреляция меньше:'
  end
  object Label3: TLabel
    Left = 164
    Top = 44
    Width = 89
    Height = 13
    Caption = 'Файл результата'
  end
  object Panel1: TPanel
    Left = 0
    Top = 206
    Width = 547
    Height = 41
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    object btStart: TSpeedButton
      Left = 8
      Top = 6
      Width = 89
      Height = 29
      Anchors = [akLeft, akBottom]
      Caption = 'Start'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33333333333333333333EEEEEEEEEEEEEEE333FFFFFFFFFFFFF3E00000000000
        00E337777777777777F3E0F77777777770E337F33333333337F3E0F333333333
        70E337F3333F333337F3E0F33303333370E337F3337FF33337F3E0F333003333
        70E337F33377FF3337F3E0F33300033370E337F333777FF337F3E0F333000033
        70E337F33377773337F3E0F33300033370E337F33377733337F3E0F333003333
        70E337F33377333337F3E0F33303333370E337F33373333337F3E0F333333333
        70E337F33333333337F3E0FFFFFFFFFFF0E337FFFFFFFFFFF7F3E00000000000
        00E33777777777777733EEEEEEEEEEEEEEE33333333333333333}
      NumGlyphs = 2
      OnClick = btStartClick
    end
    object btStop: TSpeedButton
      Left = 104
      Top = 6
      Width = 89
      Height = 29
      Anchors = [akLeft, akBottom]
      Caption = 'Stop'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33333333333333333333EEEEEEEEEEEEEEE333FFFFFFFFFFFFF3E00000000000
        00E337777777777777F3E0F77777777770E337F33333333337F3E0F333333333
        70E337F33333333337F3E0F33333333370E337F333FFFFF337F3E0F330000033
        70E337F3377777F337F3E0F33000003370E337F3377777F337F3E0F330000033
        70E337F3377777F337F3E0F33000003370E337F3377777F337F3E0F330000033
        70E337F33777773337F3E0F33333333370E337F33333333337F3E0F333333333
        70E337F33333333337F3E0FFFFFFFFFFF0E337FFFFFFFFFFF7F3E00000000000
        00E33777777777777733EEEEEEEEEEEEEEE33333333333333333}
      NumGlyphs = 2
      OnClick = btStopClick
    end
    object btSave: TSpeedButton
      Left = 200
      Top = 6
      Width = 89
      Height = 29
      Anchors = [akLeft, akBottom]
      Caption = 'Save'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000130B0000130B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00333333333333
        33333333333FFFFFFFFF333333000000000033333377777777773333330FFFFF
        FFF03333337F333333373333330FFFFFFFF03333337F3FF3FFF73333330F00F0
        00F03333F37F773777373330330FFFFFFFF03337FF7F3F3FF3F73339030F0800
        F0F033377F7F737737373339900FFFFFFFF03FF7777F3FF3FFF70999990F00F0
        00007777777F7737777709999990FFF0FF0377777777FF37F3730999999908F0
        F033777777777337F73309999990FFF0033377777777FFF77333099999000000
        3333777777777777333333399033333333333337773333333333333903333333
        3333333773333333333333303333333333333337333333333333}
      NumGlyphs = 2
      OnClick = btSaveClick
    end
    object btGraph: TSpeedButton
      Left = 296
      Top = 6
      Width = 89
      Height = 29
      Anchors = [akLeft, akBottom]
      Caption = 'Corr. graph'
      Glyph.Data = {
        76010000424D7601000000000000760000002800000020000000100000000100
        04000000000000010000120B0000120B00001000000000000000000000000000
        800000800000008080008000000080008000808000007F7F7F00BFBFBF000000
        FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF00337333733373
        3373337F3F7F3F7F3F7F33737373737373733F7F7F7F7F7F7F7F770000000000
        000077777777777777773303333333333333337FF333333F33333709333333C3
        333337773F3FF373F333330393993C3C33333F7F7F77F7F7FFFF77079797977C
        77777777777777777777330339339333C333337FF73373F37F33370C333C3933
        933337773F3737F37FF33303C3C33939C9333F7F7F7FF7F777FF7707C7C77797
        7C97777777777777777733033C3333333C33337F37F33333373F37033C333333
        33C3377F37333333337333033333333333333F7FFFFFFFFFFFFF770777777777
        7777777777777777777733333333333333333333333333333333}
      NumGlyphs = 2
      OnClick = btGraphClick
    end
    object btClose: TBitBtn
      Left = 451
      Top = 6
      Width = 89
      Height = 29
      Anchors = [akRight, akBottom]
      TabOrder = 0
      Kind = bkClose
    end
  end
  object edBitLen: TEdit
    Left = 96
    Top = 8
    Width = 77
    Height = 21
    TabOrder = 1
    Text = '13'
  end
  object Panel2: TPanel
    Left = 0
    Top = 64
    Width = 547
    Height = 142
    Align = alBottom
    Anchors = [akLeft, akTop, akRight, akBottom]
    BevelOuter = bvLowered
    TabOrder = 2
    object RxDBGrid1: TRxDBGrid
      Left = 1
      Top = 1
      Width = 545
      Height = 140
      Align = alClient
      DataSource = DataSource1
      TabOrder = 0
      TitleFont.Charset = DEFAULT_CHARSET
      TitleFont.Color = clWindowText
      TitleFont.Height = -11
      TitleFont.Name = 'MS Sans Serif'
      TitleFont.Style = []
      TitleButtons = True
      OnTitleBtnClick = RxDBGrid1TitleBtnClick
      Columns = <
        item
          Expanded = False
          FieldName = 'fldCod'
          Title.Alignment = taCenter
          Visible = True
        end
        item
          Expanded = False
          FieldName = 'fldCorrMin'
          Title.Alignment = taCenter
          Visible = True
        end
        item
          Expanded = False
          FieldName = 'fldCorrMax'
          Title.Alignment = taCenter
          Visible = True
        end
        item
          Expanded = False
          FieldName = 'fldCrossCorManMin'
          Title.Alignment = taCenter
          Visible = True
        end
        item
          Expanded = False
          FieldName = 'fldCrossCorManMax'
          Title.Alignment = taCenter
          Visible = True
        end>
    end
  end
  object StatusBar1: TStatusBar
    Left = 0
    Top = 247
    Width = 547
    Height = 20
    Panels = <
      item
        Width = 200
      end
      item
        Width = 50
      end>
    SimplePanel = False
  end
  object edMinCorr: TEdit
    Left = 348
    Top = 8
    Width = 77
    Height = 21
    TabOrder = 4
    Text = '13'
  end
  object FilenameEdit1: TFilenameEdit
    Left = 256
    Top = 40
    Width = 173
    Height = 22
    NumGlyphs = 1
    TabOrder = 5
  end
  object FormStorage1: TFormStorage
    StoredProps.Strings = (
      'FilenameEdit1.Text'
      'edBitLen.Text'
      'edMinCorr.Text')
    StoredValues = <>
    Left = 212
    Top = 176
  end
  object MemData: TRxMemoryData
    Active = True
    FieldDefs = <
      item
        Name = 'fldCod'
        DataType = ftInteger
      end
      item
        Name = 'fldCorrMin'
        DataType = ftInteger
      end
      item
        Name = 'fldCorrMax'
        DataType = ftInteger
      end
      item
        Name = 'fldCrossCorManMin'
        DataType = ftInteger
      end
      item
        Name = 'fldCrossCorManMax'
        DataType = ftInteger
      end>
    Left = 204
    Top = 128
    object fldCod: TIntegerField
      DisplayLabel = 'Код'
      FieldName = 'fldCod'
    end
    object fldCorrMin: TIntegerField
      DisplayLabel = 'Корр.мин.'
      FieldName = 'fldCorrMin'
    end
    object fldCorrMax: TIntegerField
      DisplayLabel = 'Корр.макс.'
      FieldName = 'fldCorrMax'
    end
    object fldCrossCorManMin: TIntegerField
      DisplayLabel = 'Кроскорр.мин.'
      FieldName = 'fldCrossCorManMin'
    end
    object fldCrossCorManMax: TIntegerField
      DisplayLabel = 'Кросскорр.макс.'
      FieldName = 'fldCrossCorManMax'
    end
  end
  object DataSource1: TDataSource
    DataSet = MemData
    Left = 92
    Top = 134
  end
end
