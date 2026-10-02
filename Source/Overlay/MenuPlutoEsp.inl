	static void RenderPlutoMainEsp(const ImVec2& contentSize, const ImVec2& col3, const ImVec2& spacing)
	{
		const float cw = col3.x;
		const ImGuiWindowFlags pf = MenuTheme::ScrollPanelFlags();

		ImGui::BeginGroup();
		{
			ImGui::BeginChild(true, MenuLag("ESP Switch Config", Languages).c_str(), "o", MenuTheme::StyledPanel(cw, 4, 0), false, pf);
			{
				ImGui::CheckboxWishTips(Languages == 1 ? U8("透视开关") : "ESP Switch", &GameData.Config.ESP.Enable, "ESP Switch");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("载具透视") : "Vehicle ESP", &GameData.Config.Vehicle.Enable, "Vehicle ESP");
				ImGui::Keybind(Languages == 1 ? U8("载具热键") : "Vehicle Hotkey", &GameData.Config.Vehicle.EnableKey);
				ImGui::CheckboxWishTips(Languages == 1 ? U8("物透开关") : "Item ESP", &GameData.Config.Item.Enable, "Item ESP");
			}
			ImGui::EndChild(true);

			ImGui::BeginChild(true, MenuLag("Match Info Config", Languages).c_str(), "o", MenuTheme::StyledPanel(cw, 6, 1), false, pf);
			{
				ImGui::BeginDisabled(true);
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家队伍") : "Team ID", &GameData.Config.ESP.TeamID, "Team ID");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家战队") : "Clan Name", &GameData.Config.ESP.ClanName, "Clan Name");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家等级") : "Level", &GameData.Config.ESP.等级, "Level");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家击杀") : "Kills", &GameData.Config.ESP.击杀, "Kills");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家伤害") : "Damage", &GameData.Config.ESP.伤害, "Damage");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("段位图标") : "Rank Icon", &GameData.Config.ESP.showico, "Rank Icon");
				ImGui::EndDisabled();
				MenuTheme::BeginFullWidthRow();
				ImGui::SliderInt1(Languages == 1 ? U8("信息大小") : "Info Size", &GameData.Config.ESP.FontSize, 10, 50, "%d px");
			}
			ImGui::EndChild(true);

			ImGui::BeginChild(true, MenuLag("Warning Misc Config", Languages).c_str(), "o", MenuTheme::StyledPanel(cw, 5, 2), false, pf);
			{
				ImGui::CheckboxWishTips(Languages == 1 ? U8("危险预警") : "Danger Warning", &GameData.Config.ESP.DangerWarning, "Danger Warning");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("被瞄射线") : "Aimed Ray", &GameData.Config.ESP.TargetedRay, "Aimed Ray");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家观战") : "Spectate", &GameData.Config.ESP.观战, "Spectate");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("开启预警") : "Radar Alert", &GameData.Config.Early.Enable, "Radar Alert");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("显示距离") : "Show Distance", &GameData.Config.Early.ShowDistance, "Show Distance");
				MenuTheme::BeginFullWidthRow();
				ImGui::SliderInt1(Languages == 1 ? U8("最大距离") : "Max Distance", &GameData.Config.Early.DistanceMax, 10, 1000, "%d M");
				ImGui::SliderInt1(Languages == 1 ? U8("图标大小") : "Icon Scale", &GameData.Config.Early.FontSize, 10, 50, "%d px");
			}
			ImGui::EndChild(true);

			ImGui::BeginChild(true, MenuLag("Visual Misc Config", Languages).c_str(), "o", MenuTheme::StyledPanel(cw, 4, 2), false, pf);
			{
				MenuTheme::BeginFullWidthRow();
				const char* HealthBarPositionItems[] = { Languages == 1 ? U8("顶部显示") : "Top Display", Languages == 1 ? U8("左侧显示") : "Left Display" };
				ImGui::SetNextItemWidth(-1);
				ImGui::Combo_popup(Languages == 1 ? U8("血条位置") : "Health Bar Position", &GameData.Config.ESP.血条位置, HealthBarPositionItems, IM_ARRAYSIZE(HealthBarPositionItems));
				const char* HealthBarStyleItems[] = { Languages == 1 ? U8("彩虹血条") : "Rainbow Health Bar", Languages == 1 ? U8("单色血条") : "Monochrome Health Bar", Languages == 1 ? U8("单色缩放") : "Monochrome Scaling", Languages == 1 ? U8("彩虹缩放") : "Rainbow Scaling" };
				ImGui::Combo_popup(Languages == 1 ? U8("血条样式") : "Health Bar Style", &GameData.Config.ESP.血条样式, HealthBarStyleItems, IM_ARRAYSIZE(HealthBarStyleItems));
				ImGui::CheckboxWishTips(Languages == 1 ? U8("文字阴影") : "Text Shadow", &GameData.Config.ESP.Stroke, "Text Shadow");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("可视检测") : "Visibility Check", &GameData.Config.ESP.VisibleCheck, "Visibility Check");
				ImGui::Keybind(Languages == 1 ? U8("显示队友") : "Show Teammates", &GameData.Config.ESP.duiyouKey);
			}
			ImGui::EndChild(true);
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		{
			ImGui::BeginChild(true, MenuLag("Player ESP Config", Languages).c_str(), "o", MenuTheme::StyledPanel(cw, 15, 4), false, pf);
			{
				MenuTheme::BeginFullWidthRow();
				ImGui::SliderInt1(Languages == 1 ? U8("透视距离") : "ESP Distance", &GameData.Config.ESP.DistanceMax, 0, 1000, "%d M");
				ImGui::SliderInt1(Languages == 1 ? U8("信息距离") : "Info Distance", &GameData.Config.ESP.InfoDistanceMax, 0, 1000, "%d M");
				ImGui::SliderFloat1(Languages == 1 ? U8("骨骼粗细") : "Bone Thickness", &GameData.Config.ESP.SkeletonWidth, 1.f, 5.f, "%.1f mm");
				ImGui::SliderInt1(Languages == 1 ? U8("射线粗细") : "Ray Thickness", &GameData.Config.ESP.RayWidth, 1, 5, "%d mm");

				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家骨骼") : "Bones", &GameData.Config.ESP.Skeleton, "Bones");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("头部骨骼") : "Head Bones", &GameData.Config.ESP.HeadDrawing, "Head Bones");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家方框") : "Player Box", &GameData.Config.ESP.DisplayFrame, "Player Box");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("显示射线") : "Show Ray", &GameData.Config.ESP.PlayerLine, "Show Ray");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家血条") : "Health Bar", &GameData.Config.ESP.health_bar, "Health Bar");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家名称") : "Player Name", &GameData.Config.ESP.Nickname, "Player Name");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家距离") : "Distance", &GameData.Config.ESP.Dis, "Distance");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家血量") : "Health", &GameData.Config.ESP.Health, "Health");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家手持") : "Weapon", &GameData.Config.ESP.Weapon, "Weapon");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("玩家弹药") : "Ammo", &GameData.Config.ESP.Ammo, "Ammo");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("离线显示") : "Offline", &GameData.Config.ESP.Offline, "Offline");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("倒地显示") : "Downed", &GameData.Config.ESP.Downed, "Downed");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("漏哪变色") : "Hitbox Highlight", &GameData.Config.ESP.AdjustableDistance, "Hitbox Highlight");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("锁定变色") : "Lock Color Change", &GameData.Config.ESP.suodingbianse, "Lock Color Change");
				ImGui::CheckboxWishTips(Languages == 1 ? U8("合作者") : "Partner", &GameData.Config.ESP.Partner, "Partner");
			}
			ImGui::EndChild(true);
		}
		ImGui::EndGroup();

		ImGui::SameLine();

		ImGui::BeginGroup();
		{
			ImGui::BeginChild(true, MenuLag("Match Color Config", Languages).c_str(), "o", MenuTheme::StyledPanel(cw, 12, 0), false, pf);
			{
				ImGui::ColorEdit5(Languages == 1 ? U8("可视骨骼") : "Visible Bones", GameData.Config.ESP.Color.Visible.Skeleton, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("可视信息") : "Visible Info", GameData.Config.ESP.Color.Visible.Info, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("掩体骨骼") : "Cover Bones", GameData.Config.ESP.Color.Default.Skeleton, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("掩体信息") : "Cover Info", GameData.Config.ESP.Color.Default.Info, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("人机骨骼") : "Bot Bones", GameData.Config.ESP.Color.AI.Skeleton, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("危险骨骼") : "Danger Bones", GameData.Config.ESP.Color.Dangerous.Skeleton, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("倒地骨骼") : "Downed Bones", GameData.Config.ESP.Color.Groggy.Skeleton, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("射线颜色") : "Ray Color", GameData.Config.ESP.Color.Ray.Line, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("锁定颜色") : "Lock Color", GameData.Config.ESP.Color.aim.Skeleton, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("血条颜色") : "Health Bar Color", GameData.Config.ESP.Color.xuetiaoyanse.Skeleton, picker_flags);
				ImGui::ColorEdit5(Languages == 1 ? U8("左上角信息") : "Top Left Color", GameData.Config.ESP.Color.Info.Skeleton, picker_flags);
			}
			ImGui::EndChild(true);
		}
		ImGui::EndGroup();
	}
