{
	"patcher" : 	{
		"fileversion" : 1,
		"appversion" : 		{
			"major" : 8,
			"minor" : 5,
			"revision" : 2,
			"architecture" : "x64",
			"modernui" : 1
		}
,
		"classnamespace" : "box",
		"rect" : [ 1173.0, 337.0, 642.0, 610.0 ],
		"bglocked" : 0,
		"openinpresentation" : 0,
		"default_fontsize" : 12.0,
		"default_fontface" : 0,
		"default_fontname" : "Arial",
		"gridonopen" : 1,
		"gridsize" : [ 15.0, 15.0 ],
		"gridsnaponopen" : 1,
		"objectsnaponopen" : 1,
		"statusbarvisible" : 2,
		"toolbarvisible" : 1,
		"lefttoolbarpinned" : 0,
		"toptoolbarpinned" : 0,
		"righttoolbarpinned" : 0,
		"bottomtoolbarpinned" : 0,
		"toolbars_unpinned_last_save" : 0,
		"tallnewobj" : 0,
		"boxanimatetime" : 200,
		"enablehscroll" : 1,
		"enablevscroll" : 1,
		"devicewidth" : 0.0,
		"description" : "",
		"digest" : "",
		"tags" : "",
		"style" : "",
		"subpatcher_template" : "",
		"assistshowspatchername" : 0,
		"boxes" : [ 			{
				"box" : 				{
					"fontface" : 0,
					"fontname" : "Arial",
					"fontsize" : 12.0,
					"id" : "obj-14",
					"maxclass" : "number~",
					"maximum" : 1.0,
					"minimum" : 0.0,
					"mode" : 1,
					"numinlets" : 2,
					"numoutlets" : 2,
					"outlettype" : [ "signal", "float" ],
					"patching_rect" : [ 180.0, 62.0, 56.0, 22.0 ],
					"sig" : 1.0
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-13",
					"maxclass" : "scope~",
					"numinlets" : 2,
					"numoutlets" : 0,
					"patching_rect" : [ 190.0, 359.0, 130.0, 130.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-12",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 337.406015037593988, 115.0, 31.0, 22.0 ],
					"text" : "sig~"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-11",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 337.406015037593988, 62.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-9",
					"maxclass" : "newobj",
					"numinlets" : 2,
					"numoutlets" : 1,
					"outlettype" : [ "float" ],
					"patching_rect" : [ 344.578947368421041, 254.0, 74.0, 22.0 ],
					"text" : "snapshot~ 1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-8",
					"maxclass" : "toggle",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "int" ],
					"parameter_enable" : 0,
					"patching_rect" : [ 344.578947368421041, 286.0, 24.0, 24.0 ]
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-6",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 127.0, 359.0, 45.0, 22.0 ],
					"text" : "dac~ 2"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-5",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 0,
					"patching_rect" : [ 67.0, 359.0, 45.0, 22.0 ],
					"text" : "dac~ 1"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-4",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 111.714285714285722, 62.0, 45.0, 22.0 ],
					"text" : "adc~ 2"
				}

			}
, 			{
				"box" : 				{
					"id" : "obj-3",
					"maxclass" : "newobj",
					"numinlets" : 1,
					"numoutlets" : 1,
					"outlettype" : [ "signal" ],
					"patching_rect" : [ 54.0, 62.0, 45.0, 22.0 ],
					"text" : "adc~ 1"
				}

			}
, 			{
				"box" : 				{
					"autosave" : 1,
					"id" : "obj-2",
					"inletInfo" : 					{
						"IOInfo" : [ 							{
								"type" : "signal",
								"index" : 1,
								"tag" : "in1",
								"comment" : "AudioL"
							}
, 							{
								"type" : "signal",
								"index" : 2,
								"tag" : "in2",
								"comment" : "AudioR"
							}
, 							{
								"type" : "signal",
								"index" : 3,
								"tag" : "in3",
								"comment" : "cv1"
							}
, 							{
								"type" : "signal",
								"index" : 4,
								"tag" : "in4",
								"comment" : "cv2"
							}
, 							{
								"type" : "signal",
								"index" : 5,
								"tag" : "in5",
								"comment" : "cv3"
							}
, 							{
								"type" : "signal",
								"index" : 6,
								"tag" : "in6",
								"comment" : "cv4"
							}
, 							{
								"type" : "signal",
								"index" : 7,
								"tag" : "in7",
								"comment" : "cv5"
							}
, 							{
								"type" : "signal",
								"index" : 8,
								"tag" : "in8",
								"comment" : "cv6"
							}
, 							{
								"type" : "signal",
								"index" : 9,
								"tag" : "in9",
								"comment" : "cv7"
							}
, 							{
								"type" : "signal",
								"index" : 10,
								"tag" : "in10",
								"comment" : "cv8"
							}
, 							{
								"type" : "signal",
								"index" : 11,
								"tag" : "in11",
								"comment" : "t1"
							}
, 							{
								"type" : "signal",
								"index" : 12,
								"tag" : "in12",
								"comment" : "t2"
							}
, 							{
								"type" : "signal",
								"index" : 13,
								"tag" : "in13",
								"comment" : "t3"
							}
, 							{
								"type" : "signal",
								"index" : 14,
								"tag" : "in14",
								"comment" : "t4"
							}
, 							{
								"type" : "signal",
								"index" : 15,
								"tag" : "in15",
								"comment" : "switch1"
							}
 ]
					}
,
					"maxclass" : "newobj",
					"numinlets" : 15,
					"numoutlets" : 20,
					"outletInfo" : 					{
						"IOInfo" : [ 							{
								"type" : "signal",
								"index" : 1,
								"tag" : "out1",
								"comment" : "AudioL"
							}
, 							{
								"type" : "signal",
								"index" : 2,
								"tag" : "out2",
								"comment" : "AudioR"
							}
, 							{
								"type" : "signal",
								"index" : 3,
								"tag" : "out3",
								"comment" : "cv1"
							}
, 							{
								"type" : "signal",
								"index" : 4,
								"tag" : "out4",
								"comment" : "cv2"
							}
, 							{
								"type" : "signal",
								"index" : 5,
								"tag" : "out5",
								"comment" : "cv3"
							}
, 							{
								"type" : "signal",
								"index" : 6,
								"tag" : "out6",
								"comment" : "cv4"
							}
, 							{
								"type" : "signal",
								"index" : 7,
								"tag" : "out7",
								"comment" : "cv5"
							}
, 							{
								"type" : "signal",
								"index" : 8,
								"tag" : "out8",
								"comment" : "cv6"
							}
, 							{
								"type" : "signal",
								"index" : 9,
								"tag" : "out9",
								"comment" : "cv7"
							}
, 							{
								"type" : "signal",
								"index" : 10,
								"tag" : "out10",
								"comment" : "cv8"
							}
, 							{
								"type" : "signal",
								"index" : 11,
								"tag" : "out11",
								"comment" : "t1"
							}
, 							{
								"type" : "signal",
								"index" : 12,
								"tag" : "out12",
								"comment" : "t2"
							}
, 							{
								"type" : "signal",
								"index" : 13,
								"tag" : "out13",
								"comment" : "t3"
							}
, 							{
								"type" : "signal",
								"index" : 14,
								"tag" : "out14",
								"comment" : "t4"
							}
, 							{
								"type" : "signal",
								"index" : 15,
								"tag" : "out15",
								"comment" : "led1"
							}
, 							{
								"type" : "signal",
								"index" : 16,
								"tag" : "out16",
								"comment" : "led2"
							}
, 							{
								"type" : "signal",
								"index" : 17,
								"tag" : "out17",
								"comment" : "led3"
							}
, 							{
								"type" : "signal",
								"index" : 18,
								"tag" : "out18",
								"comment" : "led4"
							}
, 							{
								"type" : "signal",
								"index" : 19,
								"tag" : "out19",
								"comment" : "ledpwm"
							}
 ]
					}
,
					"outlettype" : [ "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "signal", "list" ],
					"patching_rect" : [ 110.26315789473685, 189.0, 337.0, 22.0 ],
					"rnboattrcache" : 					{

					}
,
					"rnboversion" : "1.0.2",
					"saved_attribute_attributes" : 					{
						"valueof" : 						{
							"parameter_invisible" : 1,
							"parameter_longname" : "rnbo~",
							"parameter_shortname" : "rnbo~",
							"parameter_type" : 3
						}

					}
,
					"saved_object_attributes" : 					{
						"optimization" : "O1",
						"parameter_enable" : 1,
						"uuid" : "d78ce52f-7d8e-11ed-a52f-6e9de1f20473"
					}
,
					"snapshot" : 					{
						"filetype" : "C74Snapshot",
						"version" : 2,
						"minorversion" : 0,
						"name" : "snapshotlist",
						"origin" : "rnbo~",
						"type" : "list",
						"subtype" : "Undefined",
						"embed" : 1,
						"snapshot" : 						{
							"__presetid" : "salt"
						}
,
						"snapshotlist" : 						{
							"current_snapshot" : 0,
							"entries" : [ 								{
									"filetype" : "C74Snapshot",
									"version" : 2,
									"minorversion" : 0,
									"name" : "salt",
									"origin" : "salt",
									"type" : "rnbo",
									"subtype" : "",
									"embed" : 0,
									"snapshot" : 									{
										"__presetid" : "salt"
									}
,
									"fileref" : 									{
										"name" : "salt",
										"filename" : "salt.maxsnap",
										"filepath" : "~/Documents/Max 8/Snapshots",
										"filepos" : -1,
										"snapshotfileid" : "3d51df387ccfd02a289a6c655c3572f4"
									}

								}
 ]
						}

					}
,
					"text" : "rnbo~ salt",
					"varname" : "rnbo~"
				}

			}
 ],
		"lines" : [ 			{
				"patchline" : 				{
					"destination" : [ "obj-12", 0 ],
					"source" : [ "obj-11", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 10 ],
					"source" : [ "obj-12", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 2 ],
					"source" : [ "obj-14", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-13", 0 ],
					"source" : [ "obj-2", 2 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-5", 0 ],
					"source" : [ "obj-2", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-6", 0 ],
					"source" : [ "obj-2", 1 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-9", 0 ],
					"source" : [ "obj-2", 14 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 0 ],
					"source" : [ "obj-3", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-2", 1 ],
					"source" : [ "obj-4", 0 ]
				}

			}
, 			{
				"patchline" : 				{
					"destination" : [ "obj-8", 0 ],
					"source" : [ "obj-9", 0 ]
				}

			}
 ],
		"parameters" : 		{
			"obj-2" : [ "rnbo~", "rnbo~", 0 ],
			"parameterbanks" : 			{
				"0" : 				{
					"index" : 0,
					"name" : "",
					"parameters" : [ "-", "-", "-", "-", "-", "-", "-", "-" ]
				}

			}
,
			"inherited_shortname" : 1
		}
,
		"dependency_cache" : [ 			{
				"name" : "salt.maxsnap",
				"bootpath" : "~/Documents/Max 8/Snapshots",
				"patcherrelativepath" : "../../../../Documents/Max 8/Snapshots",
				"type" : "mx@s",
				"implicit" : 1
			}
, 			{
				"name" : "salt.rnbopat",
				"bootpath" : "~/projects/BelaPatches/rnbo/rnbo_patch",
				"patcherrelativepath" : ".",
				"type" : "RBOP",
				"implicit" : 1
			}
 ],
		"autosave" : 0
	}

}
