package com.cubic.e3box;

import java.util.ArrayList;

public class BoxStateMachine {
	public static final int STATE_END = -1;
	public static final int STATE_BEGIN = 0;	
	
	private int EVT_ERROR = -1;
	private int mCurrentState;
	
	public BoxStateMachine() {
		// TODO Auto-generated constructor stub
		mCurrentState = STATE_BEGIN;
	}
	
	private class Table{
		public int state_from;
		public int state_to;
		public int evt;
		public IAction action;
		
		public Table(int state_from, int state_to, int evt, IAction action){
			this.state_from = state_from;
			this.state_to = state_to;
			this.evt = evt;
			this.action = action;
		}
		
		public int performAction(byte[] value){
			int result = -1;
			if(action != null){
				if(action.performAction(value)){
					return this.state_to;
				}
			}else{
				return this.state_to;
			}
			return result;
		}
		
		public boolean tableEquals(Table table){
			if(this.state_from == table.state_from
					&& this.evt == table.evt){
				return true;
			}else{
				return false;
			}
		}
		
		
	}
    
	private ArrayList<Table> list = new ArrayList<BoxStateMachine.Table>();
	
	public int getState(){
		return mCurrentState;
	}
	
	public boolean insertAction(int state_from, int state_to, int evt, IAction action){
		Table table = new Table(state_from, state_to, evt, action);
		Table temp;
		
		for(int i = 0; i < list.size(); i++){
			temp = list.get(i);
			if(temp != null && temp == table){
				return false;
			}
		}
		
		list.add(table);
		return true;
	}
	
	public int inputEvent(int evt, byte[] value){
		Table table = new Table(mCurrentState, STATE_END, evt, null);
		int i;
		int result;
		Table temp;

		for(i = 0; i < list.size(); i++){
			temp = list.get(i);
			if(temp.tableEquals(table)){
				result = temp.performAction(value);
				if(result < 0){
					return -1;
				}
				mCurrentState = result;
				break;
			}
		}
		
		return mCurrentState;
		} 
}
